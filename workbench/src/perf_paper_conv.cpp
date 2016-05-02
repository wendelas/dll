//=======================================================================
// Copyright (c) 2014-2016 Baptiste Wicht
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include <iostream>
#include <chrono>

#include "dll/conv_rbm.hpp"

#include "mnist/mnist_reader.hpp"
#include "mnist/mnist_utils.hpp"

namespace {

constexpr const std::size_t EPOCHS = 5;

using clock      = std::chrono::steady_clock;
using time_point = std::chrono::time_point<clock>;
using resolution = std::chrono::milliseconds;

struct perf_timer {
    std::string name;
    std::size_t repeat;

    time_point start;

    perf_timer(std::string name, std::size_t repeat) : name(name), repeat(repeat) {
        start = clock::now();
    }

    ~perf_timer(){
        auto end      = clock::now();
        auto duration = std::chrono::duration_cast<resolution>(end - start).count();

        std::cout << name << ": " << duration / double(repeat) << "ms" << std::endl;
    }
};

#define MEASURE(rbm, name, data)                                                           \
    {                                                                                      \
        std::size_t d_min = std::numeric_limits<std::size_t>::max();                       \
        std::size_t d_max = 0;                                                             \
        for (std::size_t i = 0; i < EPOCHS; ++i) {                                         \
            time_point start = clock::now();                                               \
            rbm.train<false>(data, 1);                                                     \
            time_point end = clock::now();                                                 \
            std::size_t d  = std::chrono::duration_cast<resolution>(end - start).count();  \
            d_min          = std::min(d_min, d);                                           \
            d_max          = std::max(d_max, d);                                           \
        }                                                                                  \
        std::cout << name << ": min:" << d_min << "ms max:" << d_max << "ms" << std::endl; \
    }

} //end of anonymous namespace

int main(int argc, char* argv []) {
    auto dataset = mnist::read_dataset_direct<std::vector, etl::fast_dyn_matrix<float, 1, 28, 28>>();
    dataset.training_images.resize(10000);

    std::string number;
    if(argc > 1){
        number = argv[1];
    }

    auto n = dataset.training_images.size();

    mnist::binarize_dataset(dataset);

    decltype(auto) data_1 = dataset.training_images;
    std::vector<etl::fast_dyn_matrix<float, 40, 20, 20>> data_2(n);
    std::vector<etl::fast_dyn_matrix<float, 40, 16, 16>> data_3(n);
    std::vector<etl::fast_dyn_matrix<float, 96, 12, 12>> data_4(n);

    for(std::size_t i = 0; i < n; ++i){
        data_2[i] = etl::normal_generator() * 255.0;
        data_3[i] = etl::normal_generator() * 255.0;
        data_4[i] = etl::normal_generator() * 255.0;
    }

    mnist::binarize_each(data_2);
    mnist::binarize_each(data_3);
    mnist::binarize_each(data_4);

    cpp_assert(data_1[0].size() == 784, "Invalid input size");
    cpp_assert(data_2[0].size() == 40 * 20 * 20, "Invalid input size");
    cpp_assert(data_3[0].size() == 40 * 16 * 16, "Invalid input size");
    cpp_assert(data_4[0].size() == 96 * 12 * 12, "Invalid input size");

    std::cout << n << " images used for training" << std::endl;

    if(number.empty() || number == "1"){
        dll::conv_rbm_desc_square<1, 28, 40, 20, dll::parallel_mode, dll::serial, dll::batch_size<1>, dll::weight_type<float>>::layer_t crbm_1;
        dll::conv_rbm_desc_square<40, 20, 40, 16, dll::parallel_mode, dll::serial, dll::batch_size<1>, dll::weight_type<float>>::layer_t crbm_2;
        dll::conv_rbm_desc_square<40, 16, 96, 12, dll::parallel_mode, dll::serial, dll::batch_size<1>, dll::weight_type<float>>::layer_t crbm_3;
        dll::conv_rbm_desc_square<96, 12, 8, 8, dll::parallel_mode, dll::serial, dll::batch_size<1>, dll::weight_type<float>>::layer_t crbm_4;

        crbm_1.learning_rate = 1e-4;
        crbm_2.learning_rate = 1e-4;
        crbm_3.learning_rate = 1e-4;
        crbm_4.learning_rate = 1e-4;

        MEASURE(crbm_1, "crbm_1x28x28_normal", data_1);
        MEASURE(crbm_2, "crbm_40x20x20_normal", data_2);
        MEASURE(crbm_3, "crbm_40x16x16_normal", data_3);
        MEASURE(crbm_4, "crbm_100x12x12_normal", data_4);
    }

    return 0;
}