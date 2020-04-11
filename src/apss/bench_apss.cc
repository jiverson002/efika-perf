// SPDX-License-Identifier: MIT
#include <chrono>
#include <fstream>
#include <iostream>
#include <new>
#include <random>
#include <stdexcept>

#include "celero/Celero.h"

#include "efika/apss.h"
#include "efika/core.h"
#include "efika/data.h"
#include "efika/io.h"

namespace apss {

#ifdef HAS_BRUTEFORCE
struct bruteforce {
  int operator()(EFIKA_val_t const minsim, EFIKA_Matrix const * const M,
                 EFIKA_Matrix const * const I, EFIKA_Matrix * const S,
                 Vector * const A)
  {
    return EFIKA_apss_bruteforce(minsim, M, S);
    (void)I;
    (void)A;
  }
};
#endif

#ifdef HAS_SFRKD
struct sfrkd {
  int operator()(EFIKA_val_t const minsim, EFIKA_Matrix const * const M,
                 EFIKA_Matrix const * const I, EFIKA_Matrix * const S,
                 Vector * const A)
  {
    return EFIKA_apss_sfrkd(minsim, M, I, A);
    (void)S;
  }
};
#endif

} // apss

namespace {

class Experiment {
  public:
    Experiment(float t, std::string f)
      : T(t), file(f) { }

    virtual ~Experiment() = default;

    float T;
    std::string file;

  protected:
    Experiment() = default;
};

static std::vector<Experiment> E;

template <typename apss>
class TestFixture : public celero::TestFixture {
  private:
    EFIKA_val_t minsim_;
    EFIKA_Matrix M_;
    EFIKA_Matrix I_;
    EFIKA_Matrix S_;
    Vector A_;

  public:
    TestFixture() {
      if (!E.size()) {
        auto ndatasets = sizeof(EFIKA_datasets)/sizeof(*EFIKA_datasets);
        std::cout << "Prob. Space:" << std::endl;
        for(decltype(ndatasets) i = 0; i < ndatasets; i++) {
          auto const &filename = std::string(EFIKA_DATA_PATH) + "/" + EFIKA_datasets[i];
          std::cout << "  " << i << ": " << "{ t: 0.10, filename: \"" << filename << "\" }" << std::endl;
          E.push_back({ 0.10, filename });
        }
      }
    }

    virtual std::vector<celero::TestFixture::ExperimentValue>
    getExperimentValues() const override {
      std::vector<celero::TestFixture::ExperimentValue> problemSpace;

      for(decltype(E.size()) i = 0; i < E.size(); i++)
        problemSpace.push_back(static_cast<std::int64_t>(i));

      return problemSpace;
    }

    virtual void
    setUp(const celero::TestFixture::ExperimentValue& ex) override {
      int err;

      const auto& [ t, filename ] = E[ex.Value];

      minsim_ = t;

      err = EFIKA_Matrix_init(&M_);
      if (err)
        throw std::runtime_error("Could not initialize matrix");

      FILE * fp = fopen(filename.c_str(), "r");
      if (!fp)
        throw std::invalid_argument("Cannot open `" + filename + "' for reading");

      err = EFIKA_IO_cluto_load(fp, &M_);
      if (err)
        throw std::runtime_error("Could not load `" + filename + "'");

      fclose(fp);

      err = EFIKA_Matrix_comp(&M_);
      if (err)
        throw std::runtime_error("Could not compact matrix");

      err = EFIKA_Matrix_norm(&M_);
      if (err)
        throw std::runtime_error("Could not normalize matrix");

      err = EFIKA_Matrix_sort(&M_, EFIKA_ASC | EFIKA_COL);
      if (err)
        throw std::runtime_error("Could not sort matrix");

      err = EFIKA_Matrix_init(&I_);
      if (err)
        throw std::runtime_error("Could not initialize matrix");

      err = EFIKA_Matrix_iidx(&M_, &I_);
      if (err)
        throw std::runtime_error("Could not transpose matrix");

      err = EFIKA_Matrix_sort(&I_, EFIKA_ASC | EFIKA_VAL);
      if (err)
        throw std::runtime_error("Could not sort matrix");

      err = EFIKA_Matrix_init(&S_);
      if (err)
        throw std::runtime_error("Could not initialize solution matrix");

      A_ = vector_new();
    }

    virtual void tearDown() override {
      EFIKA_Matrix_free(&M_);
      EFIKA_Matrix_free(&I_);
      EFIKA_Matrix_free(&S_);
      vector_delete(&A_);
    }

    void TestBody() {
      apss()(minsim_, &M_, &I_, &S_, &A_);
      celero::DoNotOptimizeAway(A_.size);
    }
};

} // namespace

#ifdef HAS_BRUTEFORCE
BASELINE_F(apss, bruteforce, TestFixture<apss::bruteforce>, 2, 2) { TestBody(); }
#endif
#ifdef HAS_SFRKD
//BENCHMARK_F(apss, sfrkd, TestFixture<apss::sfrkd>, 5, 5) { TestBody(); }
#endif
