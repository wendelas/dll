//=======================================================================
// Copyright (c) 2014-2016 Baptiste Wicht
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include <numeric>

#include "catch.hpp"

#include "cpp_utils/data.hpp"

#include "dll/conv_rbm.hpp"

#include "mnist/mnist_reader.hpp"
#include "mnist/mnist_utils.hpp"

TEST_CASE("crbm/mnist_1", "crbm::simple") {
    dll::conv_rbm_desc_square<
        1, 28, 40, 12,
        dll::batch_size<25>>::layer_t rbm;

    auto dataset = mnist::read_dataset<std::vector, std::vector, double>(250);

    REQUIRE(!dataset.training_images.empty());

    mnist::binarize_dataset(dataset);

    auto error = rbm.train(dataset.training_images, 100);

    REQUIRE(error < 2e-2);
}

TEST_CASE("crbm/mnist_2", "crbm::momentum") {
    dll::conv_rbm_desc_square<
        1, 28, 40, 12,
        dll::batch_size<25>,
        dll::momentum>::layer_t rbm;

    auto dataset = mnist::read_dataset<std::vector, std::vector, double>();

    REQUIRE(!dataset.training_images.empty());
    dataset.training_images.resize(100);

    mnist::binarize_dataset(dataset);

    auto error = rbm.train(dataset.training_images, 100);

    REQUIRE(error < 1e-2);
}

TEST_CASE("crbm/mnist_3", "crbm::decay_l1") {
    dll::conv_rbm_desc_square<
        1, 28, 40, 12,
        dll::batch_size<25>,
        dll::weight_decay<dll::decay_type::L1_FULL>>::layer_t rbm;

    auto dataset = mnist::read_dataset<std::vector, std::vector, double>();

    REQUIRE(!dataset.training_images.empty());
    dataset.training_images.resize(100);

    mnist::binarize_dataset(dataset);

    auto error = rbm.train(dataset.training_images, 100);

    REQUIRE(error < 1e-2);
}

TEST_CASE("crbm/mnist_4", "crbm::decay_l2") {
    dll::conv_rbm_desc_square<
        1, 28, 40, 12,
        dll::batch_size<25>,
        dll::weight_decay<dll::decay_type::L2_FULL>>::layer_t rbm;

    auto dataset = mnist::read_dataset<std::vector, std::vector, double>();

    REQUIRE(!dataset.training_images.empty());
    dataset.training_images.resize(100);

    mnist::binarize_dataset(dataset);

    auto error = rbm.train(dataset.training_images, 100);

    REQUIRE(error < 1e-2);
}

TEST_CASE("crbm/mnist_6", "crbm::gaussian") {
    dll::conv_rbm_desc_square<
        1, 28, 20, 24,
        dll::batch_size<20>,
        dll::momentum,
        dll::weight_decay<dll::decay_type::L2>,
        dll::visible<dll::unit_type::GAUSSIAN>>::layer_t rbm;

    rbm.learning_rate /= 2;

    auto dataset = mnist::read_dataset<std::vector, std::vector, double>(200);
    REQUIRE(!dataset.training_images.empty());

    mnist::normalize_dataset(dataset);

    auto error = rbm.train(dataset.training_images, 100);

    REQUIRE(error < 2e-2);
}

TEST_CASE("crbm/mnist_10", "crbm::pcd") {
    dll::conv_rbm_desc_square<
        1, 28, 40, 12,
        dll::batch_size<25>,
        dll::momentum,
        dll::trainer_rbm<dll::pcd1_trainer_t>>::layer_t rbm;

    rbm.learning_rate *= 0.01;

    auto dataset = mnist::read_dataset<std::vector, std::vector, double>();

    REQUIRE(!dataset.training_images.empty());
    dataset.training_images.resize(100);

    mnist::binarize_dataset(dataset);

    auto error = rbm.train(dataset.training_images, 100);

    REQUIRE(error < 5e-2);
}

TEST_CASE("crbm/mnist_13", "crbm::multi_channel") {
    dll::conv_rbm_desc_square<
        2, 28, 40, 12,
        dll::batch_size<25>,
        dll::momentum>::layer_t rbm;

    auto dataset = mnist::read_dataset<std::vector, std::vector, double>();

    REQUIRE(!dataset.training_images.empty());
    dataset.training_images.resize(200);

    mnist::binarize_dataset(dataset);

    for (auto& image : dataset.training_images) {
        image.reserve(image.size() * 2);
        auto end = image.size();
        for (std::size_t i = 0; i < end; ++i) {
            image.push_back(image[i]);
        }
    }

    auto error = rbm.train(dataset.training_images, 100);

    REQUIRE(error < 1e-2);
}

TEST_CASE("crbm/mnist_14", "crbm::fast") {
    dll::conv_rbm_desc_square<
        2, 28, 40, 12,
        dll::batch_size<25>,
        dll::momentum>::layer_t rbm;

    auto dataset = mnist::read_dataset<std::vector, std::vector, double>(200);

    REQUIRE(!dataset.training_images.empty());

    mnist::binarize_dataset(dataset);

    for (auto& image : dataset.training_images) {
        image.reserve(image.size() * 2);
        auto end = image.size();
        for (std::size_t i = 0; i < end; ++i) {
            image.push_back(image[i]);
        }
    }

    auto error = rbm.train(dataset.training_images, 25);

    REQUIRE(error < 1e-2);
}

TEST_CASE("crbm/mnist_15", "crbm::denoising") {
    dll::conv_rbm_desc_square<
        1, 28, 40, 12,
        dll::batch_size<25>,
        dll::momentum,
        dll::weight_decay<dll::decay_type::L2>,
        dll::visible<dll::unit_type::GAUSSIAN>,
        dll::shuffle>::layer_t rbm;

    rbm.learning_rate *= 2;

    auto dataset = mnist::read_dataset<std::vector, std::vector, double>(200);

    REQUIRE(!dataset.training_images.empty());

    mnist::normalize_dataset(dataset);

    auto noisy = dataset.training_images;

    std::default_random_engine rand_engine(56);
    std::normal_distribution<double> normal_distribution(0.0, 0.1);
    auto noise = std::bind(normal_distribution, rand_engine);

    for (auto& image : noisy) {
        for (auto& noisy_x : image) {
            noisy_x += noise();
        }
    }

    cpp::normalize_each(noisy);

    auto error = rbm.train_denoising(noisy, dataset.training_images, 100);

    REQUIRE(error < 2e-2);
}

TEST_CASE("crbm/mnist_16", "crbm::momentum") {
    dll::conv_rbm_desc_square<
        1, 28, 40, 12,
        dll::batch_size<25>,
        dll::momentum,
        dll::parallel_mode>::layer_t rbm;

    auto dataset = mnist::read_dataset<std::vector, std::vector, double>();

    REQUIRE(!dataset.training_images.empty());
    dataset.training_images.resize(100);

    mnist::binarize_dataset(dataset);

    auto error = rbm.train(dataset.training_images, 100);

    REQUIRE(error < 2e-2);
}
