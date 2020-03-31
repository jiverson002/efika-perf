// SPDX-License-Identifier: MIT
#include <chrono>
#include <fstream>
#include <iostream>
#include <new>
#include <random>
#include <stdexcept>

#include "celero/Celero.h"

#include "efika/data.h"
#include "efika/impl.h"

namespace impl {

struct sfr1d {
  void operator()(Problem const P, Vector * const A) {
    EFIKA_Impl_sfr1d(P, A);
  }
};

#ifdef HAS_SFRKD
struct sfrkd {
  void operator()(Problem const P, Vector * const A) {
    EFIKA_Impl_sfrkd(P, A);
  }
};
#endif

} // impl

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

template <typename Impl>
class TestFixture : public celero::TestFixture {
  private:
    Problem P;
    Vector A;

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
      unsigned n, k;

      const auto& [ t, filename ] = E[ex.Value];

      std::ifstream file(filename);
      if (!file.is_open())
        throw std::invalid_argument("Cannot open `" + filename + "' for reading");

      file >> n >> k;
      if (file.fail())
        throw std::invalid_argument("Cannot read `" + filename);

      auto const mem = new float[n][5];
      if (!mem)
        throw std::bad_alloc();

      for (unsigned i = 0; i < n; i++) {
        for (unsigned j = 0; j < k; j++) {
          file >> mem[i][j];
          if (file.fail())
            throw std::invalid_argument("Cannot read `" + filename);
        }
      }

      P = { t, k, n, mem };

      A = vector_new();
    }

    virtual void tearDown() override {
      vector_delete(&A);
      delete [] P.mem;
    }

    void TestBody() {
      Impl()(P, &A);
      celero::DoNotOptimizeAway(A.size);
    }
};

} // namespace

#ifdef HAS_SFR1D
BASELINE_F (impl, sfr1d, TestFixture<impl::sfr1d>, 2, 2) { TestBody(); }
#endif
#ifdef HAS_SFRKD
BENCHMARK_F(impl, sfrkd, TestFixture<impl::sfrkd>, 5, 5) { TestBody(); }
#endif
