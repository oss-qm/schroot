/* sbuild-chroot-plain - sbuild simple chroot object
 *
 * Copyright © 2005  Roger Leigh <rleigh@debian.org>
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

/**
 * SECTION:sbuild-chroot-plain
 * @short_description: simple chroot object
 * @title: ChrootPlain
 *
 * This object represents a chroot located on a mounted filesystem.
 */

#include <config.h>

#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <unistd.h>

#include "sbuild-i18n.h"
#include "sbuild-chroot-plain.h"
#include "sbuild-keyfile.h"

using namespace sbuild;

ChrootPlain::ChrootPlain():
  Chroot(),
  location()
{
}

ChrootPlain::ChrootPlain (const keyfile&     keyfile,
			  const std::string& group):
  Chroot(keyfile, group),
  location()
{
}

ChrootPlain::~ChrootPlain()
{
}

Chroot *
ChrootPlain::clone () const
{
  return new ChrootPlain(*this);
}

/**
 * sbuild_chroot_plain_get_location:
 * @chroot: an #ChrootPlain
 *
 * Get the directory location of the chroot.
 *
 * Returns a string. This string points to internally allocated
 * storage in the chroot and must not be freed, modified or stored.
 */
const std::string&
ChrootPlain::get_location () const
{
  return this->location;
}

/**
 * sbuild_chroot_plain_set_location:
 * @chroot: an #ChrootPlain.
 * @location: the location to set.
 *
 * Set the directory location of a chroot.
 */
void
ChrootPlain::set_location (const std::string& location)
{
  this->location = location;
}

const std::string&
ChrootPlain::get_mount_location () const
{
  return this->location;
}

const std::string&
ChrootPlain::get_chroot_type () const
{
  static const std::string type("plain");

  return type;
}

void
ChrootPlain::setup_env (env_list& env)
{
  this->Chroot::setup_env(env);

  setup_env_var(env, "CHROOT_LOCATION",
		get_location());
}

void
ChrootPlain::setup_lock (Chroot::SetupType type,
			 bool              lock)
{
  /* By default, plain chroots do no locking. */
}

Chroot::SessionFlags
ChrootPlain::get_session_flags () const
{
  return static_cast<SessionFlags>(0);
}

void
ChrootPlain::print_details (std::ostream& stream) const
{
  this->Chroot::print_details(stream);

  if (!this->location.empty())
    stream << format_detail_string(_("Location"), get_location());
  stream << std::flush;
}

void
ChrootPlain::print_config (std::ostream& stream) const
{
  this->Chroot::print_config(stream);

  stream << "location=" << get_location() << '\n';
  stream << std::flush;
}

void
ChrootPlain::read_keyfile (const keyfile&     keyfile,
			   const std::string& group)
{
  std::string location;
  if (keyfile.get_value(group, "location", location))
    set_location(location);
}

/*
 * Local Variables:
 * mode:C++
 * End:
 */