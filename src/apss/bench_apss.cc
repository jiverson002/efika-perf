// SPDX-License-Identifier: MIT
#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <sstream>

#include "celero/Celero.h"

#include "efika/apss.h"
#include "efika/core.h"
#include "efika/io.h"

namespace apss {

#define EFIKA_API(impl)\
  struct impl {\
    int static pp(EFIKA_val_t const minsim, EFIKA_Matrix * const M)\
    {\
      return EFIKA_apss_ ## impl ## _pp(minsim, M);\
    }\
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
#ifdef HAS_NOVA
EFIKA_API(nova);
#endif

} // apss

namespace {

template <class Container>
void split(const std::string& str, Container& cont, char delim = ',')
{
    std::stringstream ss(str);
    std::string token;
    while (std::getline(ss, token, delim))
      cont.push_back(token);
}

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

      char const * const efika_preprocess = std::getenv("EFIKA_APSS_PREPROCESS");
      std::transform(efika_preprocess,
                     efika_preprocess + strlen(efika_preprocess),
                     std::back_inserter(preprocess_),
                     ::tolower);
    }

    virtual std::vector<std::shared_ptr<celero::UserDefinedMeasurement>>
    getUserDefinedMeasurements() const override {
      return { ncandUDM_, nprunUDM_, nvdotUDM_, nmacs1UDM_, nmacs2UDM_,
               /*nsimsUDM_*/ };
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

      if (preprocess_ == "true") {
        err = apss::pp(minsim_, &M_);
        if (err)
          throw std::runtime_error("Could not preprocess matrix");
      }

      err = EFIKA_Matrix_init(&S_);
      if (err)
        throw std::runtime_error("Could not initialize solution matrix");
    }

    virtual void tearDown() override {
      EFIKA_Matrix_free(&M_);
      EFIKA_Matrix_free(&S_);

      ncandUDM_->addValue(EFIKA_apss_ncand);
      nprunUDM_->addValue(EFIKA_apss_nprun);
      nvdotUDM_->addValue(EFIKA_apss_nvdot);
      nmacs1UDM_->addValue(EFIKA_apss_nmacs1);
      nmacs2UDM_->addValue(EFIKA_apss_nmacs2);
      nsimsUDM_->addValue(EFIKA_apss_nsims);
    }

    virtual void UserBenchmark() override {
      apss::run(minsim_, &M_, &S_);
      celero::DoNotOptimizeAway(S_.nnz);
    }

  private:
    EFIKA_val_t minsim_;
    char const * dataset_;
    std::string preprocess_;
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

void apss_main() {
  std::map<std::string, std::shared_ptr<::celero::Factory>> baseline_map;
  std::map<std::string, std::shared_ptr<::celero::Factory>> benchmark_map;

#define REGISTER_TYPE(map, type)\
  map[#type] = \
    std::make_shared<::celero::GenericFactory<TestFixture<apss::type>>>();\

#ifdef HAS_BRUTEFORCE
  REGISTER_TYPE(baseline_map, bruteforce);
#endif
#ifdef HAS_IDXJOIN
  REGISTER_TYPE(baseline_map, idxjoin);
#endif

#ifdef HAS_ALLPAIRS
  REGISTER_TYPE(benchmark_map, allpairs);
#endif
#ifdef HAS_L2AP
  REGISTER_TYPE(benchmark_map, l2ap);
#endif
#ifdef HAS_MMJOIN
  REGISTER_TYPE(benchmark_map, mmjoin);
#endif
#ifdef HAS_NOVA
  REGISTER_TYPE(benchmark_map, nova);
#endif

#ifdef HAS_IDXJOIN
  if (baseline_map.find("idxjoin") != baseline_map.end()) {
    ::celero::RegisterBaseline("apss", "idxjoin", 1, 1, 1,
        baseline_map["idxjoin"]);
  } else {
#endif
#if HAS_BRUTEFORCE
    ::celero::RegisterBaseline("apss", "bruteforce", 1, 1, 1,
      baseline_map["bruteforce"]);
#endif
  }

  char const * const efika_samples = std::getenv("EFIKA_APSS_SAMPLES");
  if (!efika_samples)
    throw std::runtime_error("Environment variable EFIKA_APSS_SAMPLES was not specified");

  char const * const efika_iterations = std::getenv("EFIKA_APSS_ITERATIONS");
  if (!efika_iterations)
    throw std::runtime_error("Environment variable EFIKA_APSS_ITERATIONS was not specified");

  std::vector<std::string> algorithm;
  const char * const efika_algorithm = std::getenv("EFIKA_APSS_ALGORITHM");
  if (efika_algorithm)
    split(efika_algorithm, algorithm);

  if (!algorithm.empty()) {
    for (const auto &name : algorithm) {
      const auto &iter = benchmark_map.find(name);
      if (iter != benchmark_map.end()) {
        ::celero::RegisterTest("apss", iter->first.c_str(),
            std::atoi(efika_samples), std::atoi(efika_iterations), 1,
            iter->second);
      } else {
        std::cerr << name << " is not implemented" << std::endl;
      }
    }
  } else {
    for (const auto &[key, val] : benchmark_map)
      if ("bruteforce" != key)
        ::celero::RegisterTest("apss", key.c_str(), std::atoi(efika_samples),
            std::atoi(efika_iterations), 1, val);
  }
}
