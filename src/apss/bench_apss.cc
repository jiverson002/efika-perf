// SPDX-License-Identifier: MIT
#include <cstdlib>
#include <iostream>
#include <memory>
#include <stdexcept>

#include "celero/Celero.h"

#include "efika/apss.h"
#include "efika/core.h"
#include "efika/io.h"

namespace apss {

#define EFIKA_API(impl)\
  struct impl {\
    static int pp(EFIKA_val_t const minsim, EFIKA_Matrix * const M) {\
      return EFIKA_apss_ ## impl ## _pp(minsim, M);\
    }\
\
    int static run(EFIKA_val_t const minsim, EFIKA_Matrix * const M,\
                   EFIKA_Matrix * const S)\
    {\
      return EFIKA_apss_ ## impl(minsim, M, S);\
    }\
  }

#ifdef HAS_ALLPAIRS
EFIKA_API(allpairs);
#endif
#ifdef HAS_BRUTEFORCE
EFIKA_API(bruteforce);
#endif
#ifdef HAS_IDXJOIN
EFIKA_API(idxjoin);
#endif
#ifdef HAS_L2AP
EFIKA_API(l2ap);
#endif
#ifdef HAS_MMJOIN
EFIKA_API(mmjoin);
#endif

} // apss

namespace {

template <typename apss>
class TestFixture : public celero::TestFixture {
  private:
    class CounterUDM : public celero::UserDefinedMeasurementTemplate<unsigned long> {
      public:
        CounterUDM(std::string &&name) : name_(name) { }

        virtual std::string getName() const override
        {
          return name_;
        }

        // Optionally turn off some statistical reporting.
        virtual bool reportMean()              const override { return false; }
        virtual bool reportSize()              const override { return false; }
        virtual bool reportVariance()          const override { return false; }
        virtual bool reportStandardDeviation() const override { return false; }
        virtual bool reportSkewness()          const override { return false; }
        virtual bool reportKurtosis()          const override { return false; }
        virtual bool reportZScore()            const override { return false; }
        virtual bool reportMax()               const override { return false; }

      private:
        std::string name_;
    };

  public:
    TestFixture() {
      char const * const efika_minsim = std::getenv("EFIKA_APSS_MINSIM");
      if (!efika_minsim)
        throw std::runtime_error("Environment variable EFIKA_APSS_MINSIM was not specified");
      minsim_ = std::strtod(efika_minsim, nullptr);

      char const * const efika_dataset = std::getenv("EFIKA_APSS_DATASET");
      if (!efika_dataset)
        throw std::runtime_error("Environment variable EFIKA_APSS_DATASET was not specified");
      dataset_ = efika_dataset;
    }

    virtual std::vector<std::shared_ptr<celero::UserDefinedMeasurement>>
    getUserDefinedMeasurements() const override {
      return { ncandUDM_, nprunUDM_, nvdotUDM_, nmacs1UDM_, nmacs2UDM_,
               /*nsimsUDM_*/ };
    }

    virtual void
    onExperimentStart(const celero::TestFixture::ExperimentValue&) override {
      int err = EFIKA_Matrix_init(&S_);
      if (err)
        throw std::runtime_error("Could not initialize solution matrix");
    }

    virtual void onExperimentEnd() override {
      EFIKA_Matrix_free(&S_);

      ncandUDM_->addValue(EFIKA_apss_ncand);
      nprunUDM_->addValue(EFIKA_apss_nprun);
      nvdotUDM_->addValue(EFIKA_apss_nvdot);
      nmacs1UDM_->addValue(EFIKA_apss_nmacs1);
      nmacs2UDM_->addValue(EFIKA_apss_nmacs2);
      nsimsUDM_->addValue(EFIKA_apss_nsims);
    }

    virtual void
    setUp(const celero::TestFixture::ExperimentValue&) override {
      int err = EFIKA_Matrix_init(&M_);
      if (err)
        throw std::runtime_error("Could not initialize matrix");

      err = EFIKA_IO_cluto_load(dataset_, &M_);
      if (err)
        throw std::runtime_error("Could not load `" + std::string(dataset_) + "'");

      err = EFIKA_Matrix_comp(&M_);
      if (err)
        throw std::runtime_error("Could not compact matrix");

      err = EFIKA_Matrix_norm(&M_);
      if (err)
        throw std::runtime_error("Could not normalize matrix");

      err = apss::pp(minsim_, &M_);
      if (err)
        throw std::runtime_error("Could not preprocess matrix");
    }

    virtual void tearDown() override {
      EFIKA_Matrix_free(&M_);
    }

    void TestBody() {
      apss::run(minsim_, &M_, &S_);
      celero::DoNotOptimizeAway(S_.nnz);
    }

  private:
    EFIKA_val_t minsim_;
    char const * dataset_;
    EFIKA_Matrix M_;
    EFIKA_Matrix S_;

    std::shared_ptr<CounterUDM> ncandUDM_  { new CounterUDM("ncand")  };
    std::shared_ptr<CounterUDM> nprunUDM_  { new CounterUDM("nprun")  };
    std::shared_ptr<CounterUDM> nvdotUDM_  { new CounterUDM("nvdot")  };
    std::shared_ptr<CounterUDM> nmacs1UDM_ { new CounterUDM("nmacs1") };
    std::shared_ptr<CounterUDM> nmacs2UDM_ { new CounterUDM("nmacs2") };
    std::shared_ptr<CounterUDM> nsimsUDM_  { new CounterUDM("nsims")  };
};

} // namespace

#if defined(HAS_IDXJOIN)
BASELINE_F(apss, idxjoin, TestFixture<apss::idxjoin>, 2, 2) { TestBody(); }
#elif defined(HAS_BRUTEFORCE)
BASELINE_F(apss, bruteforce, TestFixture<apss::bruteforce>, 2, 2) { TestBody(); }
#endif

#ifdef HAS_ALLPAIRS
BENCHMARK_F(apss, allpairs, TestFixture<apss::allpairs>, 5, 5) { TestBody(); }
#endif
#ifdef HAS_L2AP
BENCHMARK_F(apss, l2ap, TestFixture<apss::l2ap>, 5, 5) { TestBody(); }
#endif
#ifdef HAS_MMJOIN
BENCHMARK_F(apss, mmjoin, TestFixture<apss::mmjoin>, 5, 5) { TestBody(); }
#endif
#ifdef HAS_SFRKD
BENCHMARK_F(apss, sfrkd, TestFixture<apss::sfrkd>, 5, 5) { TestBody(); }
#endif
