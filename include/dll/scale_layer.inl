//=======================================================================
// Copyright (c) 2014-2016 Baptiste Wicht
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef DLL_SCALE_LAYER_INL
#define DLL_SCALE_LAYER_INL

#include "neural_base.hpp"

namespace dll {

//TODO This is not as generic as it should be
//It only supports fast_matrix input otherwise the size of input and
//output will be different and it will throw an assert One easy
//solution would be to extend the support for empty matrix in ETL

/*!
 * \brief Simple scaling layer
 */
template <typename Desc>
struct scale_layer : neural_base<scale_layer<Desc>> {
    using desc = Desc;

    static constexpr const int A = desc::A;
    static constexpr const int B = desc::B;

    scale_layer() = default;

    static std::string to_short_string() {
        return "scale";
    }

    static void display() {
        std::cout << to_short_string() << std::endl;
    }

    template <typename Input, typename Output>
    static void activate_hidden(Output& output, const Input& input) {
        output = input * (double(A) / double(B));
    }

    template <typename Input, typename Output>
    static void batch_activate_hidden(Output& output, const Input& input) {
        output = input * (double(A) / double(B));
    }

    template <typename I, typename O_A>
    static void activate_many(const I& input, O_A& h_a) {
        for (std::size_t i = 0; i < input.size(); ++i) {
            activate_one(input[i], h_a[i]);
        }
    }

    template <typename Input>
    static std::vector<Input> prepare_output(std::size_t samples) {
        return std::vector<Input>(samples);
    }

    template <typename Input>
    static Input prepare_one_output() {
        return {};
    }
};

} //end of dll namespace

#endif
