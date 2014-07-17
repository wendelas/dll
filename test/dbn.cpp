#include <deque>

#include "catch.hpp"

#include "dll/rbm.hpp"
#include "dll/dbn.hpp"

#include "mnist/mnist_reader.hpp"
#include "mnist/mnist_utils.hpp"

TEST_CASE( "dbn/mnist_1", "dbn::simple" ) {
    typedef dll::dbn<
        dll::layer<28 * 28, 100, dll::in_dbn, dll::momentum, dll::batch_size<25>, dll::init_weights>,
        dll::layer<100, 200, dll::in_dbn, dll::momentum, dll::batch_size<25>>,
        dll::layer<200, 10, dll::in_dbn, dll::momentum, dll::batch_size<25>, dll::hidden<dll::unit_type::SOFTMAX>>> dbn_t;

    auto dataset = mnist::read_dataset<std::vector, std::vector, double>();

    REQUIRE(!dataset.training_images.empty());
    dataset.training_images.resize(200);
    dataset.training_labels.resize(200);

    mnist::binarize_dataset(dataset);

    auto dbn = make_unique<dbn_t>();

    dbn->pretrain(dataset.training_images, 5);
    auto error = dbn->fine_tune(dataset.training_images, dataset.training_labels, 5, 50);

    REQUIRE(error < 5e-2);
}

TEST_CASE( "dbn/mnist_2", "dbn::containers" ) {
    typedef dll::dbn<
        dll::layer<28 * 28, 100, dll::in_dbn, dll::momentum, dll::batch_size<25>, dll::init_weights>,
        dll::layer<100, 200, dll::in_dbn, dll::momentum, dll::batch_size<25>>,
        dll::layer<200, 10, dll::in_dbn, dll::momentum, dll::batch_size<25>, dll::hidden<dll::unit_type::SOFTMAX>>> dbn_t;

    auto dataset = mnist::read_dataset<std::vector, std::deque, double>();

    REQUIRE(!dataset.training_images.empty());
    dataset.training_images.resize(200);
    dataset.training_labels.resize(200);

    mnist::binarize_dataset(dataset);

    auto dbn = make_unique<dbn_t>();

    dbn->pretrain(dataset.training_images, 5);
    auto error = dbn->fine_tune(dataset.training_images, dataset.training_labels, 5, 50);

    REQUIRE(error < 5e-2);
}