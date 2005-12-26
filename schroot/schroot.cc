/* schroot - securely enter a chroot
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

#include <config.h>

#include <iostream>

#include <locale.h>
#include <stdlib.h>
#include <stdio.h>

#include <boost/format.hpp>

#include <syslog.h>

#include "sbuild-i18n.h"
#include "sbuild-auth-conv-tty.h"
#include "sbuild-chroot.h"
#include "sbuild-config.h"
#include "sbuild-log.h"
#include "sbuild-session.h"
#include "sbuild-util.h"

#include "schroot-options.h"

using std::endl;
using boost::format;

/**
 * print_version:
 * @file: the file to print to
 *
 * Print version information.
 */
void
print_version (std::ostream& stream)
{
  stream << format(_("schroot (Debian sbuild) %1%\n")) % VERSION
	 << _("Written by Roger Leigh\n\n")
	 << _("Copyright (C) 2004-2005 Roger Leigh\n")
	 << _("This is free software; see the source for copying conditions.  There is NO\n"
	      "warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n")
	 << std::flush;
}

/**
 * get_chroot_options:
 * @config: an #sbuild::Config
 * @options: an #schroot::Options
 *
 * Get a list of chroots based on the specified options (--all, --chroot).
 *
 * Returns a NULL-terminated string vector (GStrv).
 */
sbuild::string_list
get_chroot_options(std::tr1::shared_ptr<sbuild::Config>& config,
		   schroot::Options&                     options)
{
  sbuild::string_list ret;

  if (options.all_chroots == true || options.all_sessions == true)
    {
      const sbuild::Config::chroot_list& list = config->get_chroots();

      for (sbuild::Config::chroot_list::const_iterator chroot = list.begin();
	   chroot != list.end();
	   ++chroot)
	{
	  if (((*chroot)->get_active() == false && options.all_chroots == false) ||
	      ((*chroot)->get_active() == true && options.all_sessions == false))
	    continue;
	  ret.push_back((*chroot)->get_name());
	}
    }
  else
    {
      sbuild::string_list invalid_chroots =
	config->validate_chroots(options.chroots);

      if (!invalid_chroots.empty())
	{
	  for (sbuild::string_list::const_iterator chroot = invalid_chroots.begin();
	       chroot != invalid_chroots.end();
	       ++chroot)
	    sbuild::log_error() << format(_("%1%: No such chroot")) % *chroot
				<< endl;
	  exit(EXIT_FAILURE);
	}
      ret = options.chroots;
    }

  return ret;
}

/**
 * main:
 * @argc: the number of arguments
 * @argv: argument vector
 *
 * Main routine.
 *
 * Returns 0 on success, 1 on failure.
 */
int
main (int   argc,
      char *argv[])
{
  setlocale (LC_ALL, "");

  bindtextdomain (GETTEXT_PACKAGE, LOCALEDIR);
  textdomain (GETTEXT_PACKAGE);

#ifndef SBUILD_DEBUG
  sbuild::debug_level = sbuild::DEBUG_CRITICAL;
#else
  sbuild::debug_level = sbuild::DEBUG_NONE;
#endif

  openlog("schroot", LOG_PID|LOG_NDELAY, LOG_AUTHPRIV);

  /* Parse command-line options into opt structure. */
  schroot::Options options(argc, argv);

  if (options.version == true)
    {
      print_version(std::cout);
      exit(EXIT_SUCCESS);
    }

  /* Initialise chroot configuration. */
  std::tr1::shared_ptr<sbuild::Config> config(new sbuild::Config);

  /* The normal chroot list is used when starting a session or running
     any chroot type or session, or displaying chroot information. */
  if (options.load_chroots == true)
    config->add_config_file(SCHROOT_CONF);
  /* The session chroot list is used when running or ending an
     existing session, or displaying chroot information. */
  if (options.load_sessions == true)
    config->add_config_directory(SCHROOT_SESSION_DIR);

  if (config->get_chroots().empty())
    {
      if (options.quiet == false)
	sbuild::log_error()
	  << format(_("No chroots are defined in %1%")) % SCHROOT_CONF
	  << endl;
      exit (EXIT_FAILURE);
    }

  /* Print chroot list (including aliases). */
  if (options.list == true)
    {
      config->print_chroot_list(std::cout);
      exit(EXIT_SUCCESS);
    }

  /* Get list of chroots to use */
  const sbuild::string_list& chroots = get_chroot_options(config, options);
  if (chroots.empty())
    {
      sbuild::log_error()
	<< format(_("The specified chroots are not defined in %1%"))
	% SCHROOT_CONF
	<< endl;
      exit (EXIT_FAILURE);
    }

  /* Print chroot information for specified chroots. */
  if (options.info == true)
    {
      config->print_chroot_info(chroots, std::cout);
      exit (EXIT_SUCCESS);
    }

  if (options.session_operation == sbuild::Session::OPERATION_BEGIN &&
      chroots.size() != 1)
    {
      sbuild::log_error()
	<< _("Only one chroot may be specified when beginning a session")
	<< endl;
      exit (EXIT_FAILURE);
    }

  /* Create a session. */
  sbuild::Session session("schroot",
			  config,
			  options.session_operation,
			  chroots);
  try
    {
      if (!options.user.empty())
	session.set_user(options.user);
      if (!options.command.empty())
	session.set_command(options.command);
      if (options.preserve)
	session.set_environment(environ);
      session.set_force(options.session_force);
      sbuild::Auth::Verbosity verbosity = sbuild::Auth::VERBOSITY_NORMAL;
      if (options.quiet)
	verbosity = sbuild::Auth::VERBOSITY_QUIET;
      else if (options.verbose)
	verbosity = sbuild::Auth::VERBOSITY_VERBOSE;
      session.set_verbosity(verbosity);

      /* Set up authentication timeouts. */
      std::tr1::shared_ptr<sbuild::AuthConv> conv(new sbuild::AuthConvTty);
      time_t curtime = 0;
      time(&curtime);
      conv->set_warning_timeout(curtime + 15);
      conv->set_fatal_timeout(curtime + 20);
      session.set_conv(conv);

      /* Run session. */
      session.run();
    }
  catch (std::runtime_error& e)
    {
      sbuild::log_error()
	<< format(_("Session failure: %1%")) % e.what() << endl;
    }

  int exit_status = session.get_child_status();

  closelog();

  exit (exit_status);
}

/*
 * Local Variables:
 * mode:C++
 * End:
 */