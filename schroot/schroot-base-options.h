/* Copyright © 2005-2006  Roger Leigh <rleigh@debian.org>
 *
 * schroot is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * schroot is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA  02111-1307  USA
 *
 *********************************************************************/

#ifndef SCHROOT_BASE_OPTIONS_H
#define SCHROOT_BASE_OPTIONS_H

#include <sbuild/sbuild-session.h>
#include <sbuild/sbuild-types.h>

#include <string>

#ifdef HAVE_TR1_MEMORY
#include <tr1/memory>
#elif HAVE_BOOST_SHARED_PTR_HPP
#include <boost/shared_ptr.hpp>
namespace std { namespace tr1 { using boost::shared_ptr; } }
#else
#error A shared_ptr implementation is not available
#endif

#include <boost/program_options.hpp>

namespace schroot_base
{

  /**
   * Basic schroot command-line options.  This is specialised by the
   * frontends to suit their particular command-line options and
   * behaviour.  This class implements the functionality common to all
   * options parsing classes.
   */
  class options
  {
  public:
    /// A shared_ptr to an options object.
    typedef std::tr1::shared_ptr<options> ptr;

    /// The constructor.
    options ();

    /// The destructor.
    virtual ~options ();

    /**
     * Parse the command-line options.
     *
     * @param argc the number of arguments
     * @param argv argument vector
     */
    void
    parse (int   argc,
	   char *argv[]);

    /// Quiet messages.
    bool quiet;
    /// Verbose messages.
    bool verbose;

    /**
     * Get the visible options group.  This options group contains all
     * the options visible to the user.
     *
     * @returns the options_description.
     */
    boost::program_options::options_description const&
    get_visible_options() const;

  protected:
    /**
     * Add options to option groups.
     */
    virtual void
    add_options ();

    /**
     * Add option groups to container groups.
     */
    virtual void
    add_option_groups ();

    /**
     * Check options after parsing.
     */
    virtual void
    check_options ();

    /**
     * Check actions after parsing.
     */
    virtual void
    check_actions ();

    /// General options group.
    boost::program_options::options_description            general;
    /// Hidden options group.
    boost::program_options::options_description            hidden;
    /// Positional options group.
    boost::program_options::positional_options_description positional;
    /// Visible options container (used for --help).
    boost::program_options::options_description            visible;
    /// Global options container (used for parsing).
    boost::program_options::options_description            global;
    /// Variables map, filled during parsing.
    boost::program_options::variables_map                  vm;

  private:
    /// Debug level string.
    std::string debug_level;
  };

}

#endif /* SCHROOT_BASE_OPTIONS_H */

/*
 * Local Variables:
 * mode:C++
 * End:
 */

