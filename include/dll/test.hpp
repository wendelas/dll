//=======================================================================
// Copyright (c) 2014-2016 Baptiste Wicht
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef DLL_TEST_HPP
#define DLL_TEST_HPP

#include "cpp_utils/stop_watch.hpp"

namespace dll {

struct predictor {
    template <typename T, typename V>
    std::size_t operator()(T& dbn, V& image) {
        return dbn->predict(image);
    }
};

#ifdef DLL_SVM_SUPPORT

struct svm_predictor {
    template <typename T, typename V>
    std::size_t operator()(T& dbn, V& image) {
        return dbn->svm_predict(image);
    }
};

#endif //DLL_SVM_SUPPORT

struct deep_predictor {
    template <typename T, typename V>
    std::size_t operator()(T& dbn, V& image) {
        return dbn->deep_predict(image, 5);
    }
};

struct label_predictor {
    template <typename T, typename V>
    std::size_t operator()(T& dbn, V& image) {
        return dbn->predict_labels(image, 10);
    }
};

struct deep_label_predictor {
    template <typename T, typename V>
    std::size_t operator()(T& dbn, V& image) {
        return dbn->deep_predict_labels(image, 10, 5);
    }
};

template <typename DBN, typename Functor, typename Samples, typename Labels>
double test_set(DBN& dbn, const Samples& images, const Labels& labels, Functor&& f) {
    return test_set(dbn, images.begin(), images.end(), labels.begin(), labels.end(), std::forward<Functor>(f));
}

template <typename DBN, typename Functor, typename Iterator, typename LIterator>
double test_set(DBN& dbn, Iterator first, Iterator last, LIterator lfirst, LIterator /*llast*/, Functor&& f) {
    std::size_t success = 0;
    std::size_t images  = 0;

    while (first != last) {
        const auto& image = *first;
        const auto& label = *lfirst;

        auto predicted = f(dbn, image);

        if (predicted == label) {
            ++success;
        }

        ++images;
        ++first;
        ++lfirst;
    }

    return (images - success) / static_cast<double>(images);
}

template <typename DBN, typename Samples>
double test_set_ae(DBN& dbn, const Samples& images) {
    return test_set_ae(dbn, images.begin(), images.end());
}

template <typename DBN, typename Iterator>
double test_set_ae(DBN& dbn, Iterator first, Iterator last) {
    double rate        = 0.0;
    std::size_t images = 0;

    while (first != last) {
        decltype(auto) image = *first;
        decltype(auto) rec_image = dbn.features(image);

        rate += etl::mean(image - rec_image);

        ++images;
        ++first;
    }

    return std::abs(rate) / images;
}

} //end of dll namespace

#endif
