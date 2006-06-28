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

#include <config.h>

#include <sbuild/sbuild-i18n.h>

#include "schroot-releaselock-options.h"

#include <cstdlib>
#include <iostream>

#include <boost/format.hpp>
#include <boost/program_options.hpp>

using std::endl;
using boost::format;
namespace opt = boost::program_options;
using namespace schroot_releaselock;

options::options ():
  schroot_base::options(),
  action(ACTION_RELEASELOCK),
  device(),
  pid(0),
  lock(_("Lock"))
{
}

options::~options ()
{
}

void
options::add_options ()
{
  schroot_base::options::add_options();

  lock.add_options()
    ("device,d", opt::value<std::string>(&this->device),
     _("Device to unlock (full path)"))
    ("pid,p", opt::value<int>(&this->pid),
     _("Process ID owning the lock"));
}

void
options::add_option_groups ()
{
  schroot_base::options::add_option_groups();

  if (!lock.options().empty())
    {
      visible.add(lock);
      global.add(lock);
    }
}

void
options::check_options ()
{
  schroot_base::options::check_options();

  if (vm.count("help"))
    set_action(ACTION_HELP);

  if (vm.count("version"))
    set_action(ACTION_VERSION);

  if (this->device.empty() &&
      this->action != ACTION_HELP &&
      this->action != ACTION_VERSION)
    throw opt::validation_error(_("No device specified"));
}

void
options::set_action (action_type action)
{
  if (this->action != ACTION_RELEASELOCK)
    throw opt::validation_error(_("Only one action may be specified"));

  this->action = action;
}

/*
 * Local Variables:
 * mode:C++
 * End:
 */
