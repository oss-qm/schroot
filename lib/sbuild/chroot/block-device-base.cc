/* Copyright © 2005-2013  Roger Leigh <rleigh@debian.org>
 *
 * schroot is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * schroot is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see
 * <http://www.gnu.org/licenses/>.
 *
 *********************************************************************/

#include <config.h>

#include <sbuild/chroot/block-device-base.h>
#include <sbuild/chroot/lvm-snapshot.h>
#include <sbuild/chroot/facet/mountable.h>
#include "format-detail.h"
#include "lock.h"
#include "util.h"

#include <cerrno>
#include <cstring>

#include <boost/format.hpp>

using boost::format;
using namespace sbuild;

namespace sbuild
{
  namespace chroot
  {

    block_device_base::block_device_base ():
      chroot(),
      device()
    {
      add_facet(facet::mountable::create());
    }

    block_device_base::block_device_base
    (const block_device_base& rhs):
      chroot(rhs),
      device(rhs.device)
    {
    }

    block_device_base::~block_device_base ()
    {
    }

    std::string const&
    block_device_base::get_device () const
    {
      return this->device;
    }

    void
    block_device_base::set_device (std::string const& device)
    {
      if (!is_absname(device))
        throw error(device, DEVICE_ABS);

      this->device = device;
    }

    std::string
    block_device_base::get_path () const
    {
      facet::mountable::const_ptr pmnt
        (get_facet<facet::mountable>());

      std::string path(get_mount_location());

      if (pmnt)
        path += pmnt->get_location();

      return path;
    }

    void
    block_device_base::setup_env (chroot const& chroot,
                                  environment& env) const
    {
      chroot::setup_env(chroot, env);

      env.add("CHROOT_DEVICE", get_device());
    }

    chroot::chroot::session_flags
    block_device_base::get_session_flags (chroot const& chroot) const
    {
      return SESSION_NOFLAGS;
    }

    void
    block_device_base::get_details (chroot const& chroot,
                                    format_detail& detail) const
    {
      this->chroot::get_details(chroot, detail);

      if (!this->device.empty())
        detail.add(_("Device"), get_device());
    }

    void
    block_device_base::get_used_keys (string_list& used_keys) const
    {
      chroot::get_used_keys(used_keys);

      used_keys.push_back("device");
    }

    void
    block_device_base::get_keyfile (chroot const& chroot,
                                    keyfile&      keyfile) const
    {
      chroot::get_keyfile(chroot, keyfile);

      keyfile::set_object_value(*this, &block_device_base::get_device,
                                keyfile, get_name(), "device");
    }

    void
    block_device_base::set_keyfile (chroot&        chroot,
                                    keyfile const& keyfile)
    {
      chroot::set_keyfile(chroot, keyfile);

      keyfile::get_object_value(*this, &block_device_base::set_device,
                                keyfile, get_name(), "device",
                                keyfile::PRIORITY_REQUIRED);
    }

  }
}