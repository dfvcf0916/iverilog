#ifndef __vvp_net_sig_H
#define __vvp_net_sig_H
/*
 * Copyright (c) 2004-2009 Stephen Williams (steve@icarus.com)
 *
 *    This source code is free software; you can redistribute it
 *    and/or modify it in source code form under the terms of the GNU
 *    General Public License as published by the Free Software
 *    Foundation; either version 2 of the License, or (at your option)
 *    any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */

# include  "config.h"
# include  "vpi_user.h"
# include  "vvp_net.h"
# include  <stddef.h>
# include  <stdlib.h>
# include  <string.h>
# include  <new>
# include  <assert.h>

#ifdef HAVE_IOSFWD
# include  <iosfwd>
#else
class ostream;
#endif

using namespace std;

/* vvp_fun_signal
 * This node is the place holder in a vvp network for signals,
 * including nets of various sort. The output from a signal follows
 * the type of its port-0 input. If vvp_vector4_t values come in
 * through port-0, then vvp_vector4_t values are propagated. If
 * vvp_vector8_t values come in through port-0, then vvp_vector8_t
 * values are propagated. Thus, this node is slightly polymorphic.
 *
 * If the signal is a net (i.e. a wire or tri) then this node will
 * have an input that is the data source. The data source will connect
 * through port-0.
 *
 * If the signal is a reg, then there will be no netlist input, the
 * values will be written by behavioral statements. The %set and
 * %assign statements will write through port-0.
 *
 * In any case, behavioral code is able to read the value that this
 * node last propagated, by using the value() method. That is important
 * functionality of this node.
 *
 * Continuous assignments are made through port-1. When a value is
 * written here, continuous assign mode is activated, and input
 * through port-0 is ignored until continuous assign mode is turned
 * off again. Writing into this port can be done in behavioral code
 * using the %cassign/v instruction, or can be done by the network by
 * hooking the output of a vvp_net_t to this port.
 */


class vvp_fun_signal_base : public vvp_net_fun_t {

    public:
      vvp_fun_signal_base();

      void deassign();
      void deassign_pv(unsigned base, unsigned wid);

    public:

	/* The %cassign/link instruction needs a place to write the
	   source node of the force, so that subsequent %cassign and
	   %deassign instructions can undo the link as needed. */
      struct vvp_net_t*cassign_link;

    protected:
      bool continuous_assign_active_;
      vvp_vector2_t assign_mask_;

    protected:
	// This is true until at least one propagation happens.
      bool needs_init_;
};

/*
 * Variables and wires can have their values accessed, so this base
 * class offers the unified concept of an acessible value.
 */
class vvp_signal_value {
    public:
      virtual ~vvp_signal_value() =0;
      virtual unsigned value_size() const =0;
      virtual vvp_bit4_t value(unsigned idx) const =0;
      virtual vvp_scalar_t scalar_value(unsigned idx) const =0;
      virtual vvp_vector4_t vec4_value() const =0;
      virtual double real_value() const;

      virtual void get_signal_value(struct t_vpi_value*vp);
};

/*
 * This abstract class is a little more specific than the signal_base
 * class, in that it adds vector access methods.
 */
class vvp_fun_signal_vec : public vvp_fun_signal_base {
    public:
      virtual vvp_vector4_t vec4_unfiltered_value() const =0;
};

class automatic_signal_base : public vvp_signal_value, public vvp_net_fil_t {

    public:
	// Automatic variables cannot be forced or released. Provide
	// stubs that assert.
      virtual void release(vvp_net_ptr_t ptr, bool net_flag);
      virtual void release_pv(vvp_net_ptr_t ptr, unsigned base, unsigned wid, bool net_flag);

      virtual unsigned filter_size() const;
      virtual void force_fil_vec4(const vvp_vector4_t&val, vvp_vector2_t mask);
      virtual void force_fil_vec8(const vvp_vector8_t&val, vvp_vector2_t mask);
      virtual void force_fil_real(double val, vvp_vector2_t mask);
      virtual void get_value(struct t_vpi_value*value);
};

/*
 * Statically allocated vvp_fun_signal4.
 */
class vvp_fun_signal4_sa : public vvp_fun_signal_vec {

    public:
      explicit vvp_fun_signal4_sa(unsigned wid, vvp_bit4_t init=BIT4_X);

      void recv_vec4(vvp_net_ptr_t port, const vvp_vector4_t&bit,
                     vvp_context_t);
      void recv_vec8(vvp_net_ptr_t port, const vvp_vector8_t&bit);

	// Part select variants of above
      void recv_vec4_pv(vvp_net_ptr_t port, const vvp_vector4_t&bit,
			unsigned base, unsigned wid, unsigned vwid,
                        vvp_context_t);
      void recv_vec8_pv(vvp_net_ptr_t port, const vvp_vector8_t&bit,
			unsigned base, unsigned wid, unsigned vwid);

	// Get information about the vector value.
      vvp_vector4_t vec4_unfiltered_value() const;

    private:
      vvp_vector4_t bits4_;
};

/*
 * Automatically allocated vvp_fun_signal4.
 */
class vvp_fun_signal4_aa : public vvp_fun_signal_vec, public automatic_signal_base, public automatic_hooks_s {

    public:
      explicit vvp_fun_signal4_aa(unsigned wid, vvp_bit4_t init=BIT4_X);

      void alloc_instance(vvp_context_t context);
      void reset_instance(vvp_context_t context);
#ifdef CHECK_WITH_VALGRIND
      void free_instance(vvp_context_t context);
#endif

      void recv_vec4(vvp_net_ptr_t port, const vvp_vector4_t&bit,
                     vvp_context_t context);

	// Part select variants of above
      void recv_vec4_pv(vvp_net_ptr_t port, const vvp_vector4_t&bit,
			unsigned base, unsigned wid, unsigned vwid,
                        vvp_context_t);

	// Get information about the vector value.
      unsigned   value_size() const;
      vvp_bit4_t value(unsigned idx) const;
      vvp_scalar_t scalar_value(unsigned idx) const;
      vvp_vector4_t vec4_value() const;
      vvp_vector4_t vec4_unfiltered_value() const;

    private:
      unsigned context_idx_;
      unsigned size_;
};

class vvp_fun_signal8  : public vvp_fun_signal_vec {

    public:
      explicit vvp_fun_signal8(unsigned wid);

      void recv_vec4(vvp_net_ptr_t port, const vvp_vector4_t&bit,
                     vvp_context_t context);
      void recv_vec8(vvp_net_ptr_t port, const vvp_vector8_t&bit);

	// Part select variants of above
      void recv_vec4_pv(vvp_net_ptr_t port, const vvp_vector4_t&bit,
                        unsigned base, unsigned wid, unsigned vwid,
                        vvp_context_t context);
      void recv_vec8_pv(vvp_net_ptr_t port, const vvp_vector8_t&bit,
                        unsigned base, unsigned wid, unsigned vwid);

    private:
      vvp_vector8_t bits8_;
};

class vvp_fun_signal_real : public vvp_fun_signal_base {

    public:
      explicit vvp_fun_signal_real() {};

	// Get information about the vector value.
      virtual double real_unfiltered_value() const = 0;

      unsigned size() const { return 1; }
};

/*
 * Statically allocated vvp_fun_signal_real.
 */
class vvp_fun_signal_real_sa : public vvp_fun_signal_real {

    public:
      explicit vvp_fun_signal_real_sa();

      void recv_real(vvp_net_ptr_t port, double bit,
                     vvp_context_t);

	// Get information about the vector value.
      double real_unfiltered_value() const;

    private:
      double bits_;
};

/*
 * Automatically allocated vvp_fun_signal_real.
 */
class vvp_fun_signal_real_aa : public vvp_fun_signal_real, public automatic_signal_base, public automatic_hooks_s {

    public:
      explicit vvp_fun_signal_real_aa();

      void alloc_instance(vvp_context_t context);
      void reset_instance(vvp_context_t context);
#ifdef CHECK_WITH_VALGRIND
      void free_instance(vvp_context_t context);
#endif

      void recv_real(vvp_net_ptr_t port, double bit,
                     vvp_context_t context);

	// Get information about the vector value.
      double real_unfiltered_value() const;

	// Get information about the vector value.
      unsigned   value_size() const;
      vvp_bit4_t value(unsigned idx) const;
      vvp_scalar_t scalar_value(unsigned idx) const;
      vvp_vector4_t vec4_value() const;
      double real_value() const;
      void get_signal_value(struct t_vpi_value*vp);

    private:
      unsigned context_idx_;
};


/* vvp_wire
 * The vvp_wire is different from vvp_variable objects in that it
 * exists only as a filter. The vvp_wire class tree is for
 * implementing verilog wires/nets (as opposed to regs/variables).
 */

class vvp_wire_base  : public vvp_net_fil_t, public vvp_signal_value {

    public:
      vvp_wire_base();
      ~vvp_wire_base();
};

class vvp_wire_vec4 : public vvp_wire_base {

    public:
      vvp_wire_vec4(unsigned wid, vvp_bit4_t init);

	// The main filter behavior for this class. These methods take
	// the value that the node is driven to, and applies the firce
	// filters. In wires, this also saves the driven value, so
	// that when a force is released, we can revert to the driven value.
      prop_t filter_vec4(const vvp_vector4_t&bit, vvp_vector4_t&rep,
			 unsigned base, unsigned vwid);
      prop_t filter_vec8(const vvp_vector8_t&val, vvp_vector8_t&rep,
			 unsigned base, unsigned vwid);

	// Abstract methods from vvp_vpi_callback
      void get_value(struct t_vpi_value*value);
	// Abstract methods from vvp_net_fit_t
      unsigned filter_size() const;
      void force_fil_vec4(const vvp_vector4_t&val, vvp_vector2_t mask);
      void force_fil_vec8(const vvp_vector8_t&val, vvp_vector2_t mask);
      void force_fil_real(double val, vvp_vector2_t mask);
      void release(vvp_net_ptr_t ptr, bool net_flag);
      void release_pv(vvp_net_ptr_t ptr, unsigned base, unsigned wid, bool net_flag);

	// Implementation of vvp_signal_value methods
      unsigned value_size() const;
      vvp_bit4_t value(unsigned idx) const;
      vvp_scalar_t scalar_value(unsigned idx) const;
      vvp_vector4_t vec4_value() const;

    private:
      vvp_bit4_t filtered_value_(unsigned idx) const;

    private:
      bool needs_init_;
      vvp_vector4_t bits4_; // The tracked driven value
      vvp_vector4_t force4_; // the value being forced
};

class vvp_wire_vec8 : public vvp_wire_base {

    public:
      vvp_wire_vec8(unsigned wid);

	// The main filter behavior for this class
      prop_t filter_vec4(const vvp_vector4_t&bit, vvp_vector4_t&rep,
			 unsigned base, unsigned vwid);
      prop_t filter_vec8(const vvp_vector8_t&val, vvp_vector8_t&rep,
			 unsigned base, unsigned vwid);

	// Abstract methods from vvp_vpi_callback
      void get_value(struct t_vpi_value*value);
	// Abstract methods from vvp_net_fit_t
      unsigned filter_size() const;
      void force_fil_vec4(const vvp_vector4_t&val, vvp_vector2_t mask);
      void force_fil_vec8(const vvp_vector8_t&val, vvp_vector2_t mask);
      void force_fil_real(double val, vvp_vector2_t mask);
      void release(vvp_net_ptr_t ptr, bool net_flag);
      void release_pv(vvp_net_ptr_t ptr, unsigned base, unsigned wid, bool net_flag);

	// Implementation of vvp_signal_value methods
      unsigned value_size() const;
      vvp_bit4_t value(unsigned idx) const;
      vvp_scalar_t scalar_value(unsigned idx) const;
      vvp_vector4_t vec4_value() const;
	// This is new to vvp_wire_vec8
      vvp_vector8_t vec8_value() const;

    private:
      vvp_scalar_t filtered_value_(unsigned idx) const;

    private:
      bool needs_init_;
      vvp_vector8_t bits8_;
      vvp_vector8_t force8_; // the value being forced
};

class vvp_wire_real : public vvp_wire_base {

    public:
      explicit vvp_wire_real(void);

	// The main filter behavior for this class
      prop_t filter_real(double&bit);

	// Abstract methods from vvp_vpi_callback
      void get_value(struct t_vpi_value*value);
	// Abstract methods from vvp_net_fit_t
      unsigned filter_size() const;
      void force_fil_vec4(const vvp_vector4_t&val, vvp_vector2_t mask);
      void force_fil_vec8(const vvp_vector8_t&val, vvp_vector2_t mask);
      void force_fil_real(double val, vvp_vector2_t mask);
      void release(vvp_net_ptr_t ptr, bool net_flag);
      void release_pv(vvp_net_ptr_t ptr, unsigned base, unsigned wid, bool net_flag);

	// Implementation of vvp_signal_value methods
      unsigned value_size() const;
      vvp_bit4_t value(unsigned idx) const;
      vvp_scalar_t scalar_value(unsigned idx) const;
      vvp_vector4_t vec4_value() const;
      double real_value() const;

      void get_signal_value(struct t_vpi_value*vp);

    private:
      double bit_;
      double force_;
};

#endif
