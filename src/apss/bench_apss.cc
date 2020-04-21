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

#ifdef HAS_REF_L2AP
# include "l2ap/includes.h"
#endif

namespace apss {

namespace efika {

#define EFIKA_API(impl)\
  struct impl {\
    static void set_up(const EFIKA_val_t minsim, const std::string &filename)\
    {\
      minsim_ = minsim;\
      int err = EFIKA_Matrix_init(&M_);\
      if (err)\
        throw std::runtime_error("Could not initialize matrix");\
      err = EFIKA_IO_cluto_load(filename.c_str(), &M_);\
      if (err)\
        throw std::runtime_error("Could not load `" + filename + "'");\
      err = EFIKA_Matrix_comp(&M_);\
      if (err)\
        throw std::runtime_error("Could not compact matrix");\
      err = EFIKA_Matrix_norm(&M_);\
      if (err)\
        throw std::runtime_error("Could not normalize matrix");\
      err = EFIKA_Matrix_init(&S_);\
      if (err)\
        throw std::runtime_error("Could not initialize solution matrix");\
    }\
    static int pp()\
    {\
      return EFIKA_apss_ ## impl ## _pp(minsim_, &M_);\
    }\
    static int run()\
    {\
      return EFIKA_apss_ ## impl(minsim_, &M_, &S_);\
    }\
    static void tear_down()\
    {\
      EFIKA_Matrix_free(&M_);\
      EFIKA_Matrix_free(&S_);\
    }\
    static EFIKA_ind_t ncand()  { return EFIKA_apss_ncand;  }\
    static EFIKA_ind_t nmacs1() { return EFIKA_apss_nmacs1; }\
    static EFIKA_ind_t nmacs2() { return EFIKA_apss_nmacs2; }\
    static EFIKA_ind_t nprun()  { return EFIKA_apss_nprun;  }\
    static EFIKA_ind_t nvdot()  { return EFIKA_apss_nvdot;  }\
    private:\
    static EFIKA_val_t minsim_;\
    static EFIKA_Matrix M_;\
    static EFIKA_Matrix S_;\
  };\
  EFIKA_val_t  impl::minsim_;\
  EFIKA_Matrix impl::M_;\
  EFIKA_Matrix impl::S_

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

} // efika

namespace ref {

#define REF_API(PROJ)\
  struct PROJ {\
    static void set_up(const EFIKA_val_t minsim, const std::string &filename)\
    {\
      ::PROJ ## _set_up(minsim, filename.c_str());\
    }\
    static int pp()\
    {\
      return ::PROJ ## _pp();\
    }\
    static int run()\
    {\
      return ::PROJ();\
    }\
    static void tear_down()\
    {\
      ::PROJ ## _tear_down();\
    }\
    static EFIKA_ind_t ncand()  { return ::PROJ ## _ncand;  }\
    static EFIKA_ind_t nmacs1() { return ::PROJ ## _nmacs1; }\
    static EFIKA_ind_t nmacs2() { return ::PROJ ## _nmacs2; }\
    static EFIKA_ind_t nprun()  { return ::PROJ ## _nprun;  }\
    static EFIKA_ind_t nvdot()  { return ::PROJ ## _nvdot;  }\
  }

#ifdef HAS_REF_L2AP
REF_API(L2AP);
#endif

} // ref

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

        virtual std::string getName() const override { return name_; }

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
    virtual std::vector<std::shared_ptr<celero::UserDefinedMeasurement>>
    getUserDefinedMeasurements() const override {
      return { ncandUDM_, nprunUDM_, nvdotUDM_, nmacs1UDM_, nmacs2UDM_ };
    }

    virtual void
    setUp(const celero::TestFixture::ExperimentValue&) override {
      char const * const env_minsim = std::getenv("EFIKA_APSS_MINSIM");
      if (!env_minsim)
        throw std::runtime_error("Environment variable EFIKA_APSS_MINSIM was not specified");
      auto const minsim = std::strtod(env_minsim, nullptr);

      char const * const env_dataset = std::getenv("EFIKA_APSS_DATASET");
      if (!env_dataset)
        throw std::runtime_error("Environment variable EFIKA_APSS_DATASET was not specified");
      auto const dataset = env_dataset;

      char const * const env_preprocess = std::getenv("EFIKA_APSS_PREPROCESS");
      std::string preprocess;
      if (env_preprocess)
        std::transform(env_preprocess,
                       env_preprocess + strlen(env_preprocess),
                       std::back_inserter(preprocess),
                       ::tolower);

      apss::set_up(minsim, dataset);

      if (preprocess == "true") {
        int err = apss::pp();
        if (err)
          throw std::runtime_error("Could not preprocess matrix");
      }
    }

    virtual void tearDown() override {
      apss::tear_down();

      ncandUDM_->addValue(apss::ncand());
      nprunUDM_->addValue(apss::nprun());
      nvdotUDM_->addValue(apss::nvdot());
      nmacs1UDM_->addValue(apss::nmacs1());
      nmacs2UDM_->addValue(apss::nmacs2());
    }

    virtual void UserBenchmark() override {
      apss::run();
    }

  private:
    std::shared_ptr<CounterUDM> ncandUDM_  { new CounterUDM("ncand")  };
    std::shared_ptr<CounterUDM> nprunUDM_  { new CounterUDM("nprun")  };
    std::shared_ptr<CounterUDM> nvdotUDM_  { new CounterUDM("nvdot")  };
    std::shared_ptr<CounterUDM> nmacs1UDM_ { new CounterUDM("nmacs1") };
    std::shared_ptr<CounterUDM> nmacs2UDM_ { new CounterUDM("nmacs2") };
};

} // namespace

void apss_main() {
  std::map<std::string, std::shared_ptr<celero::Factory>> baseline_map;
  std::map<std::string, std::shared_ptr<celero::Factory>> benchmark_map;
  std::map<std::string, std::shared_ptr<celero::Factory>> reference_map;

#define REGISTER_TYPE(map, type)\
  map[#type] = \
    std::make_shared<celero::GenericFactory<TestFixture<apss::type>>>();\

#ifdef HAS_BRUTEFORCE
  REGISTER_TYPE(baseline_map, efika::bruteforce);
#endif
#ifdef HAS_IDXJOIN
  REGISTER_TYPE(baseline_map, efika::idxjoin);
#endif

#ifdef HAS_ALLPAIRS
  REGISTER_TYPE(benchmark_map, efika::allpairs);
#endif
#ifdef HAS_L2AP
  REGISTER_TYPE(benchmark_map, efika::l2ap);
#endif
#ifdef HAS_MMJOIN
  REGISTER_TYPE(benchmark_map, efika::mmjoin);
#endif
#ifdef HAS_NOVA
  REGISTER_TYPE(benchmark_map, efika::nova);
#endif

#ifdef HAS_REF_L2AP
REGISTER_TYPE(reference_map, ref::L2AP);
#endif

  if (baseline_map.find("efika::idxjoin") != baseline_map.end()) {
    celero::RegisterBaseline("apss", "efika::idxjoin", 1, 1, 1,
      baseline_map["efika::idxjoin"]);
  } else {
    celero::RegisterBaseline("apss", "efika::bruteforce", 1, 1, 1,
      baseline_map["efika::bruteforce"]);
  }

  char const * const env_samples = std::getenv("EFIKA_APSS_SAMPLES");
  if (!env_samples)
    throw std::runtime_error("Environment variable EFIKA_APSS_SAMPLES was not specified");

  char const * const env_iterations = std::getenv("EFIKA_APSS_ITERATIONS");
  if (!env_iterations)
    throw std::runtime_error("Environment variable EFIKA_APSS_ITERATIONS was not specified");

  /* register reference implementations */
  for (const auto &[key, val] : reference_map)
    celero::RegisterTest("apss", key.c_str(), std::atoi(env_samples),
      std::atoi(env_iterations), 1, val);

  /* check for benchmark implementation selections */
  std::vector<std::string> algorithm;
  const char * const env_algorithm = std::getenv("EFIKA_APSS_ALGORITHM");
  if (env_algorithm)
    split(env_algorithm, algorithm);

  /* register benchmark implementations */
  if (!algorithm.empty()) {
    for (const auto &name : algorithm) {
      const auto &iter = benchmark_map.find(std::string("efika::") + name);
      if (iter != benchmark_map.end()) {
        celero::RegisterTest("apss", iter->first.c_str(),
          std::atoi(env_samples), std::atoi(env_iterations), 1,
          iter->second);
      } else {
        std::cerr << name << " is not implemented" << std::endl;
      }
    }
  } else {
    for (const auto &[key, val] : benchmark_map)
      celero::RegisterTest("apss", key.c_str(), std::atoi(env_samples),
        std::atoi(env_iterations), 1, val);
  }
}
