//=======================================================================
// Copyright (c) 2014-2016 Baptiste Wicht
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef DLL_DYN_RBM_INL
#define DLL_DYN_RBM_INL

#include "etl/etl.hpp"

#include "standard_rbm.hpp"

namespace dll {

/*!
 * \brief Standard version of Restricted Boltzmann Machine
 *
 * This follows the definition of a RBM by Geoffrey Hinton.
 */
template <typename Desc>
struct dyn_rbm final : public standard_rbm<dyn_rbm<Desc>, Desc> {
    using desc      = Desc;
    using weight    = typename desc::weight;
    using base_type = standard_rbm<dyn_rbm<Desc>, Desc>;

    static constexpr const unit_type visible_unit = desc::visible_unit;
    static constexpr const unit_type hidden_unit  = desc::hidden_unit;

    using w_type = etl::dyn_matrix<weight>;
    using b_type = etl::dyn_vector<weight>;
    using c_type = etl::dyn_vector<weight>;

    //Weights and biases
    w_type w; //!< Weights
    b_type b; //!< Hidden biases
    c_type c; //!< Visible biases

    //Backup weights and biases
    std::unique_ptr<w_type> bak_w; //!< Backup Weights
    std::unique_ptr<b_type> bak_b; //!< Backup Hidden biases
    std::unique_ptr<c_type> bak_c; //!< Backup Visible biases

    //Reconstruction data
    etl::dyn_vector<weight> v1; //!< State of the visible units

    etl::dyn_vector<weight> h1_a; //!< Activation probabilities of hidden units after first CD-step
    etl::dyn_vector<weight> h1_s; //!< Sampled value of hidden units after first CD-step

    etl::dyn_vector<weight> v2_a; //!< Activation probabilities of visible units after first CD-step
    etl::dyn_vector<weight> v2_s; //!< Sampled value of visible units after first CD-step

    etl::dyn_vector<weight> h2_a; //!< Activation probabilities of hidden units after last CD-step
    etl::dyn_vector<weight> h2_s; //!< Sampled value of hidden units after last CD-step

    template <std::size_t B>
    using input_batch_t = etl::fast_dyn_matrix<weight, B, 1>; //This is fake, should never be used

    size_t num_visible;
    size_t num_hidden;

    size_t batch_size = 25;

    //No copying
    dyn_rbm(const dyn_rbm& rbm) = delete;
    dyn_rbm& operator=(const dyn_rbm& rbm) = delete;

    //No moving
    dyn_rbm(dyn_rbm&& rbm) = delete;
    dyn_rbm& operator=(dyn_rbm&& rbm) = delete;

    dyn_rbm()
            : standard_rbm<dyn_rbm<Desc>, Desc>() {}

    /*!
     * \brief Initialize a RBM with basic weights.
     *
     * The weights are initialized from a normal distribution of
     * zero-mean and 0.1 variance.
     */
    dyn_rbm(size_t num_visible, size_t num_hidden)
            : standard_rbm<dyn_rbm<Desc>, Desc>(),
              w(num_visible, num_hidden),
              b(num_hidden, static_cast<weight>(0.0)),
              c(num_visible, static_cast<weight>(0.0)),
              v1(num_visible),
              h1_a(num_hidden),
              h1_s(num_hidden),
              v2_a(num_visible),
              v2_s(num_visible),
              h2_a(num_hidden),
              h2_s(num_hidden),
              num_visible(num_visible),
              num_hidden(num_hidden) {
        //Initialize the weights with a zero-mean and unit variance Gaussian distribution
        w = etl::normal_generator<weight>() * 0.1;
    }

    void init_rbm(size_t nv, size_t nh) {
        num_visible = nv;
        num_hidden  = nh;

        w    = etl::dyn_matrix<weight>(num_visible, num_hidden);
        b    = etl::dyn_vector<weight>(num_hidden, static_cast<weight>(0.0));
        c    = etl::dyn_vector<weight>(num_visible, static_cast<weight>(0.0));
        v1   = etl::dyn_vector<weight>(num_visible);
        h1_a = etl::dyn_vector<weight>(num_hidden);
        h1_s = etl::dyn_vector<weight>(num_hidden);
        v2_a = etl::dyn_vector<weight>(num_visible);
        v2_s = etl::dyn_vector<weight>(num_visible);
        h2_a = etl::dyn_vector<weight>(num_hidden);
        h2_s = etl::dyn_vector<weight>(num_hidden);

        //Initialize the weights with a zero-mean and unit variance Gaussian distribution
        w = etl::normal_generator<weight>() * 0.1;
    }

    std::size_t input_size() const noexcept {
        return num_visible;
    }

    std::size_t output_size() const noexcept {
        return num_hidden;
    }

    std::size_t parameters() const noexcept {
        return num_visible * num_hidden;
    }

    static std::string to_short_string() noexcept {
        return "RBM(dyn)";
    }

    void display() const {
        std::cout << "RBM(dyn): " << num_visible << " -> " << num_hidden << std::endl;
    }

    template <bool P = true, bool S = true, typename H1, typename H2, typename V>
    void activate_hidden(H1&& h_a, H2&& h_s, const V& v_a, const V& v_s) const {
        etl::dyn_vector<weight> t(num_hidden);
        activate_hidden(std::forward<H1>(h_a), std::forward<H2>(h_s), v_a, v_s, b, w, t);
    }

    template <bool P = true, bool S = true, typename H1, typename H2, typename V, typename T>
    void activate_hidden(H1&& h_a, H2&& h_s, const V& v_a, const V& v_s, T&& t) const {
        activate_hidden(std::forward<H1>(h_a), std::forward<H2>(h_s), v_a, v_s, b, w, std::forward<T>(t));
    }

    template <bool P = true, bool S = true, typename H1, typename H2, typename V, typename B, typename W>
    void activate_hidden(H1&& h_a, H2&& h_s, const V& v_a, const V& v_s, const B& b, const W& w) const {
        etl::dyn_vector<weight> t(num_hidden);
        activate_hidden(std::forward<H1>(h_a), std::forward<H2>(h_s), v_a, v_s, b, w, t);
    }

    template <bool P = true, bool S = true, typename H1, typename H2, typename V, typename B, typename W, typename T>
    void activate_hidden(H1&& h_a, H2&& h_s, const V& v_a, const V& v_s, const B& b, const W& w, T&& t) const {
        cpp_assert(etl::size(h_a) == num_hidden, "Invalid h_a size");
        cpp_assert(etl::size(h_s) == num_hidden, "Invalid h_s size");
        cpp_assert(etl::size(v_a) == num_visible, "Invalid v_a size");
        cpp_assert(etl::size(v_s) == num_visible, "Invalid v_s size");
        cpp_assert(etl::size(t) == num_hidden, "Invalid t size");

        base_type::template std_activate_hidden(std::forward<H1>(h_a), std::forward<H2>(h_s), v_a, v_s, b, w, std::forward<T>(t));
    }

    template <bool P = true, bool S = true, typename H, typename V>
    void activate_visible(const H& h_a, const H& h_s, V&& v_a, V&& v_s) const {
        etl::dyn_vector<weight> t(num_visible);
        activate_visible(h_a, h_s, std::forward<V>(v_a), std::forward<V>(v_s), t);
    }

    template <bool P = true, bool S = true, typename H, typename V, typename T>
    void activate_visible(const H& h_a, const H& h_s, V&& v_a, V&& v_s, T&& t) const {
        cpp_assert(etl::size(h_a) == num_hidden, "Invalid h_a size");
        cpp_assert(etl::size(h_s) == num_hidden, "Invalid h_s size");
        cpp_assert(etl::size(v_a) == num_visible, "Invalid v_a size");
        cpp_assert(etl::size(v_s) == num_visible, "Invalid v_s size");
        cpp_assert(etl::size(t) == num_hidden, "Invalid t size");

        base_type::template std_activate_visible(h_a, h_s, std::forward<V>(v_a), std::forward<V>(v_s), c, w, std::forward<T>(t));
    }

    template <bool P = true, bool S = true, typename H1, typename H2, typename V>
    void batch_activate_hidden(H1&& h_a, H2&& h_s, const V& v_a, const V& v_s) const {
        base_type::template batch_std_activate_hidden<P, S>(std::forward<H1>(h_a), std::forward<H2>(h_s), v_a, v_s, b, w);
    }

    template <bool P = true, bool S = true, typename H, typename V>
    void batch_activate_visible(const H& h_a, const H& h_s, V&& v_a, V&& v_s) const {
        base_type::template batch_std_activate_visible<P, S>(h_a, h_s, std::forward<V>(v_a), std::forward<V>(v_s), c, w);
    }

    template <typename H, typename V>
    void activate_hidden(H&& h_a, const V& v_a) const {
        etl::dyn_matrix<weight, 1> t(num_hidden);
        base_type::template std_activate_hidden<true, false>(std::forward<H>(h_a), std::forward<H>(h_a), v_a, v_a, b, w, t);
    }

    template <typename H, typename V>
    void batch_activate_hidden(H&& h_a, const V& v_a) const {
        base_type::template batch_std_activate_hidden<true, false>(std::forward<H>(h_a), std::forward<H>(h_a), v_a, v_a, b, w);
    }
};

} //end of dll namespace

#endif
