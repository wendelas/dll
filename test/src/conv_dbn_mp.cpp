//=======================================================================
// Copyright (c) 2014-2016 Baptiste Wicht
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include <deque>

#include "catch.hpp"

#define DLL_SVM_SUPPORT

#include "dll/conv_rbm_mp.hpp"
#include "dll/dbn.hpp"

#include "mnist/mnist_reader.hpp"
#include "mnist/mnist_utils.hpp"

TEST_CASE("conv_dbn_mp/mnist_1", "conv_dbn::simple") {
    typedef dll::dbn_desc<
        dll::dbn_layers<
            dll::conv_rbm_mp_desc_square<1, 28, 40, 12, 2, dll::momentum, dll::batch_size<25>>::layer_t,
            dll::conv_rbm_mp_desc_square<40, 6, 20, 4, 2, dll::momentum, dll::batch_size<25>>::layer_t>>::dbn_t dbn_t;

    auto dataset = mnist::read_dataset_direct<std::vector, etl::fast_dyn_matrix<double, 1, 28, 28>>(100);

    REQUIRE(!dataset.training_images.empty());

    mnist::binarize_dataset(dataset);

    auto dbn = std::make_unique<dbn_t>();

    dbn->pretrain(dataset.training_images, 5);
}

TEST_CASE("conv_dbn_mp/mnist_2", "conv_dbn::svm_simple") {
    typedef dll::dbn_desc<
        dll::dbn_layers<
            dll::conv_rbm_mp_desc_square<1, 28, 40, 18, 2, dll::momentum, dll::batch_size<25>>::layer_t,
            dll::conv_rbm_mp_desc_square<40, 9, 40, 6, 2, dll::momentum, dll::batch_size<25>>::layer_t>>::dbn_t dbn_t;

    auto dataset = mnist::read_dataset_direct<std::vector, etl::fast_dyn_matrix<double, 1, 28, 28>>(200);

    REQUIRE(!dataset.training_images.empty());

    mnist::binarize_dataset(dataset);

    auto dbn = std::make_unique<dbn_t>();

    dbn->pretrain(dataset.training_images, 20);

    auto result = dbn->svm_train(dataset.training_images, dataset.training_labels);

    REQUIRE(result);

    auto test_error = dll::test_set(dbn, dataset.training_images, dataset.training_labels, dll::svm_predictor());
    std::cout << "test_error:" << test_error << std::endl;
    REQUIRE(test_error < 0.2);
}

TEST_CASE("conv_dbn_mp/mnist_3", "conv_dbn::svm_concatenate") {
    typedef dll::dbn_desc<
        dll::dbn_layers<
            dll::conv_rbm_mp_desc_square<1, 28, 40, 18, 2, dll::momentum, dll::batch_size<25>>::layer_t,
            dll::conv_rbm_mp_desc_square<40, 9, 40, 6, 2, dll::momentum, dll::batch_size<25>>::layer_t>,
        dll::svm_concatenate>::dbn_t dbn_t;

    auto dataset = mnist::read_dataset_direct<std::vector, etl::fast_dyn_matrix<double, 1, 28, 28>>(200);

    REQUIRE(!dataset.training_images.empty());

    mnist::binarize_dataset(dataset);

    auto dbn = std::make_unique<dbn_t>();

    dbn->pretrain(dataset.training_images, 20);

    auto result = dbn->svm_train(dataset.training_images, dataset.training_labels);

    REQUIRE(result);

    auto test_error = dll::test_set(dbn, dataset.training_images, dataset.training_labels, dll::svm_predictor());
    std::cout << "test_error:" << test_error << std::endl;
    REQUIRE(test_error < 0.2);
}

TEST_CASE("conv_dbn_mp/mnist_slow", "[cdbn][slow][benchmark]") {
    typedef dll::dbn_desc<
        dll::dbn_layers<
            dll::conv_rbm_mp_desc_square<1, 28, 40, 16, 2, dll::momentum, dll::batch_size<25>>::layer_t,
            dll::conv_rbm_mp_desc_square<40, 8, 40, 4, 2, dll::momentum, dll::batch_size<25>>::layer_t>>::dbn_t dbn_t;

    auto dataset = mnist::read_dataset_direct<std::vector, etl::fast_dyn_matrix<double, 1, 28, 28>>(250);

    mnist::binarize_dataset(dataset);

    auto dbn = std::make_unique<dbn_t>();

    dbn->pretrain(dataset.training_images, 20);
}
