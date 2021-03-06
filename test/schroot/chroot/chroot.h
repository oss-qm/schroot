/* Copyright © 2006-2013  Roger Leigh <rleigh@debian.org>
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

#ifndef TEST_SCHROOT_CHROOT_H
#define TEST_SCHROOT_CHROOT_H

#include <gtest/gtest.h>

#include <schroot/config.h>
#include <schroot/chroot/chroot.h>
#ifdef SCHROOT_FEATURE_PERSONALITY
#include <schroot/chroot/facet/personality.h>
#endif // SCHROOT_FEATURE_PERSONALITY
#include <schroot/chroot/facet/session.h>
#include <schroot/chroot/facet/session-clonable.h>
#include <schroot/chroot/facet/source.h>
#include <schroot/chroot/facet/source-clonable.h>
#ifdef SCHROOT_FEATURE_UNION
#include <schroot/chroot/facet/fsunion.h>
#endif // SCHROOT_FEATURE_UNION
#ifdef SCHROOT_FEATURE_UNSHARE
#include <schroot/chroot/facet/unshare.h>
#endif // SCHROOT_FEATURE_UNSHARE
#include <schroot/chroot/facet/userdata.h>
#include <schroot/i18n.h>
#include <schroot/types.h>
#include <schroot/util.h>

#include <algorithm>
#include <iostream>
#include <set>

using schroot::_;

class ChrootBase : public ::testing::Test
{
protected:
  std::string                  type;
  schroot::chroot::chroot::ptr chroot;
  schroot::chroot::chroot::ptr session;
  schroot::chroot::chroot::ptr source;
  schroot::chroot::chroot::ptr session_source;
#ifdef SCHROOT_FEATURE_UNION
  schroot::chroot::chroot::ptr chroot_union;
  schroot::chroot::chroot::ptr session_union;
  schroot::chroot::chroot::ptr source_union;
  schroot::chroot::chroot::ptr session_source_union;
#endif // SCHROOT_FEATURE_UNION
  std::string                  abs_testdata_dir;

public:
  ChrootBase(const std::string& type):
    type(type),
    chroot(),
    session(),
    source(),
    session_source(),
#ifdef SCHROOT_FEATURE_UNION
    chroot_union(),
    session_union(),
    source_union(),
    session_source_union(),
#endif // SCHROOT_FEATURE_UNION
    abs_testdata_dir()
  {
    abs_testdata_dir = schroot::getcwd();
    abs_testdata_dir.append("/" TESTDATADIR);
  }

  virtual ~ChrootBase()
  {}

  void SetUp()
  {
    // Create new chroot
    chroot = schroot::chroot::chroot::create(type);
    ASSERT_NE(chroot, nullptr);
    ASSERT_FALSE(static_cast<bool>(chroot->get_facet<schroot::chroot::facet::session>()));

    setup_chroot_props(chroot);

    ASSERT_GT(chroot->get_name().length(), 0);

    // Create new source chroot.
    schroot::chroot::facet::session_clonable::const_ptr psess
      (chroot->get_facet<schroot::chroot::facet::session_clonable>());
    if (psess)
      {
        session = chroot->clone_session("test-session-name",
                                                    "test-session-name",
                                                    "user1",
                                                    false);
        if (session)
          {
            ASSERT_NE(session->get_facet<schroot::chroot::facet::session>(), nullptr);
          }
      }

    schroot::chroot::facet::source_clonable::const_ptr psrc
      (chroot->get_facet<schroot::chroot::facet::source_clonable>());
    if (psrc)
      source = chroot->clone_source();
    if (source)
      {
        schroot::chroot::facet::source_clonable::const_ptr pfsrcc
          (source->get_facet<schroot::chroot::facet::source_clonable>());
        ASSERT_EQ(pfsrcc, nullptr);
        schroot::chroot::facet::source::const_ptr pfsrc
          (source->get_facet<schroot::chroot::facet::source>());
        ASSERT_NE(pfsrc, nullptr);
      }

    if (source)
      {
        schroot::chroot::facet::session_clonable::const_ptr psess_src
          (source->get_facet<schroot::chroot::facet::session_clonable>());
        if (psess_src)
          {
            session_source = source->clone_session("test-session-name",
                                                               "test-session-name",
                                                               "user1",
                                                               false);
            if (session_source)
              {
                ASSERT_NE(session_source->get_facet<schroot::chroot::facet::session>(), nullptr);
              }
          }
      }

#ifdef SCHROOT_FEATURE_UNION
    chroot_union = schroot::chroot::chroot::create(type);
    schroot::chroot::facet::fsunion::ptr un =
      chroot_union->get_facet<schroot::chroot::facet::fsunion>();
    if (!un)
      {
        chroot_union.reset();
      }
    else
      {
        un->set_union_type("aufs");

        setup_chroot_props(chroot_union);
        ASSERT_EQ(chroot_union->get_facet<schroot::chroot::facet::session>(), nullptr);
        ASSERT_GT(chroot_union->get_name().length(), 0);

        un->set_union_overlay_directory("/overlay");
        un->set_union_underlay_directory("/underlay");
        un->set_union_mount_options("union-mount-options");

        session_union =
          chroot_union->clone_session("test-union-session-name",
                                            "test-union-session-name",
                                            "user1",
                                            false);
        source_union = chroot_union->clone_source();

        schroot::chroot::facet::session_clonable::const_ptr puni_sess_src
          (source_union->get_facet<schroot::chroot::facet::session_clonable>());
        if (puni_sess_src)
          {
            session_source_union = source_union->clone_session("test-session-name",
                                                               "test-session-name",
                                                               "user1",
                                                               false);
          }

        ASSERT_NE(session_union, nullptr);
        ASSERT_NE(session_union->get_facet<schroot::chroot::facet::session>(), nullptr);
        ASSERT_NE(source_union, nullptr);
        ASSERT_NE(session_source_union, nullptr);
        ASSERT_NE(session_source_union->get_facet<schroot::chroot::facet::session>(), nullptr);
      }
#endif // SCHROOT_FEATURE_UNION

  }

  virtual void setup_chroot_props (schroot::chroot::chroot::ptr& chroot)
  {
    chroot->set_name("test-name");
    chroot->set_description("test-description");
    chroot->set_aliases(schroot::split_string("test-alias-1,test-alias-2", ","));
    chroot->set_description("test-description");
    chroot->set_mount_location("/mnt/mount-location");
    chroot->set_environment_filter(SCHROOT_DEFAULT_ENVIRONMENT_FILTER);
    chroot->set_users(schroot::split_string("user1,user2", ","));
    chroot->set_root_users(schroot::split_string("user3,user4", ","));
    chroot->set_groups(schroot::split_string("group1,group2", ","));
    chroot->set_root_groups(schroot::split_string("group3,group4", ","));
    chroot->set_verbosity("quiet");
    chroot->set_preserve_environment(false);
    chroot->set_default_shell("/bin/testshell");

#ifdef SCHROOT_FEATURE_PERSONALITY
    schroot::chroot::facet::personality::ptr pfac
      (chroot->get_facet<schroot::chroot::facet::personality>());
    if (pfac)
      pfac->set_persona(schroot::personality("undefined"));
#endif // SCHROOT_FEATURE_PERSONALITY

    schroot::chroot::facet::source_clonable::ptr usrc
      (chroot->get_facet<schroot::chroot::facet::source_clonable>());
    if (usrc)
      {
        usrc->set_source_users(schroot::split_string("suser1,suser2", ","));
        usrc->set_source_root_users(schroot::split_string("suser3,suser4", ","));
        usrc->set_source_groups(schroot::split_string("sgroup1,sgroup2", ","));
        usrc->set_source_root_groups(schroot::split_string("sgroup3,sgroup4", ","));
      }

    schroot::chroot::facet::userdata::ptr pusr
      (chroot->get_facet<schroot::chroot::facet::userdata>());
    if (pusr)
      {
        pusr->set_data("custom.test1", "testval");
        schroot::string_set userkeys;
        userkeys.insert("schroot.resolver");
        userkeys.insert("debian.dist");
        userkeys.insert("schroot.purge");
        schroot::string_set rootkeys;
        rootkeys.insert("debian.apt-update");
        pusr->set_user_modifiable_keys(userkeys);
        pusr->set_root_modifiable_keys(rootkeys);
      }
  }

  void TearDown()
  {
    chroot = schroot::chroot::chroot::ptr();
    session = schroot::chroot::chroot::ptr();
    source = schroot::chroot::chroot::ptr();
    session_source = schroot::chroot::chroot::ptr();
#ifdef SCHROOT_FEATURE_UNION
    chroot_union = schroot::chroot::chroot::ptr();
    session_union = schroot::chroot::chroot::ptr();
    source_union = schroot::chroot::chroot::ptr();
    session_source_union = schroot::chroot::chroot::ptr();
#endif // SCHROOT_FEATURE_UNION
  }

  void setup_env_chroot (schroot::environment& env)
  {
    env.add("CHROOT_NAME",           "test-name");
    env.add("SESSION_ID",            "test-name");
    env.add("CHROOT_DESCRIPTION",    "test-description");
    //    env.add("CHROOT_SCRIPT_CONFIG",  schroot::normalname(std::string(SCHROOT_SYSCONF_DIR) + "/default/config"));
    env.add("CHROOT_PROFILE",        "default");
    env.add("CHROOT_PROFILE_DIR",    schroot::normalname(std::string(SCHROOT_SYSCONF_DIR) + "/default"));
    env.add("CUSTOM_TEST1",          "testval");
    env.add("SETUP_CONFIG", "default/config");
    env.add("SETUP_COPYFILES", "default/copyfiles");
    env.add("SETUP_FSTAB", "default/fstab");
    env.add("SETUP_NSSDATABASES", "default/nssdatabases");
#ifdef SCHROOT_FEATURE_UNSHARE
    env.add("UNSHARE_NET", "false");
    env.add("UNSHARE_SYSVIPC", "false");
    env.add("UNSHARE_SYSVSEM", "false");
    env.add("UNSHARE_UTS", "false");
#endif // SCHROOT_FEATURE_UNSHARE
  }

  void setup_keyfile_chroot (schroot::keyfile&  keyfile,
                             const std::string& group)
  {
    keyfile.set_value(group, "description", "test-description");
    keyfile.set_value(group, "aliases", "test-alias-1,test-alias-2");
    keyfile.set_value(group, "users", "user1,user2");
    keyfile.set_value(group, "root-users", "user3,user4");
    keyfile.set_value(group, "groups", "group1,group2");
    keyfile.set_value(group, "root-groups", "group3,group4");
    keyfile.set_value(group, "environment-filter",
                      SCHROOT_DEFAULT_ENVIRONMENT_FILTER);
    keyfile.set_value(group, "command-prefix", "");
    keyfile.set_value(group, "profile", "default");
    keyfile.set_value(group, "message-verbosity", "quiet");
    keyfile.set_value(group, "preserve-environment", "false");
    keyfile.set_value(group, "shell", "/bin/testshell");
    keyfile.set_value(group, "user-modifiable-keys", "debian.dist,schroot.purge,schroot.resolver");
    keyfile.set_value(group, "root-modifiable-keys", "debian.apt-update");
    keyfile.set_value(group, "setup.config", "default/config");
    keyfile.set_value(group, "setup.copyfiles", "default/copyfiles");
    keyfile.set_value(group, "setup.fstab", "default/fstab");
    keyfile.set_value(group, "setup.nssdatabases", "default/nssdatabases");
    keyfile.set_value(group, "custom.test1", "testval");
#ifdef SCHROOT_FEATURE_UNSHARE
    keyfile.set_value(group, "unshare.net", "false");
    keyfile.set_value(group, "unshare.sysvipc", "false");
    keyfile.set_value(group, "unshare.sysvsem", "false");
    keyfile.set_value(group, "unshare.uts", "false");
#endif // SCHROOT_FEATURE_UNSHARE
  }

  void setup_keyfile_session (schroot::keyfile&  keyfile,
                              const std::string& group)
  {
    setup_keyfile_chroot(keyfile, group);
    keyfile.set_value(group, "original-name", "test-name");
    keyfile.set_value(group, "users", "user1");
    keyfile.set_value(group, "root-users", "");
    keyfile.set_value(group, "groups", "");
    keyfile.set_value(group, "root-groups", "");
  }

#ifdef SCHROOT_FEATURE_UNION
  void setup_keyfile_union_unconfigured (schroot::keyfile&  keyfile,
                                         const std::string& group)
  {
    keyfile.set_value(group, "union-type", "none");
  }

  void setup_keyfile_union_configured (schroot::keyfile&  keyfile,
                                       const std::string& group)
  {
    keyfile.set_value(group, "union-type", "aufs");
    keyfile.set_value(group, "union-mount-options", "union-mount-options");
    keyfile.set_value(group, "union-overlay-directory", "/overlay");
    keyfile.set_value(group, "union-underlay-directory", "/underlay");
  }

  void setup_keyfile_union_session (schroot::keyfile&  keyfile,
                                    const std::string& group)
  {
    keyfile.set_value(group, "union-type", "aufs");
    keyfile.set_value(group, "union-mount-options", "union-mount-options");
    keyfile.set_value(group, "union-overlay-directory", "/overlay/test-union-session-name");
    keyfile.set_value(group, "union-underlay-directory", "/underlay/test-union-session-name");
  }
#endif // SCHROOT_FEATURE_UNION

  void setup_keyfile_session_clone (schroot::keyfile&  keyfile,
                                    const std::string& group)
  {
    keyfile.set_value(group, "description", chroot->get_description() + ' ' + _("(session chroot)"));
    keyfile.set_value(group, "aliases", "");
  }

  void setup_keyfile_source (schroot::keyfile&  keyfile,
                             const std::string& group)
  {
    keyfile.set_value(group, "source-clone", "true");
    keyfile.set_value(group, "source-users", "suser1,suser2");
    keyfile.set_value(group, "source-root-users", "suser3,suser4");
    keyfile.set_value(group, "source-groups", "sgroup1,sgroup2");
    keyfile.set_value(group, "source-root-groups", "sgroup3,sgroup4");
  }

  void setup_keyfile_source_clone (schroot::keyfile&  keyfile,
                                   const std::string& group)
  {
    keyfile.set_value(group, "description", chroot->get_description() + ' ' + _("(source chroot)"));
    keyfile.set_value(group, "users", "suser1,suser2");
    keyfile.set_value(group, "root-users", "suser3,suser4");
    keyfile.set_value(group, "groups", "sgroup1,sgroup2");
    keyfile.set_value(group, "root-groups", "sgroup3,sgroup4");
    keyfile.set_value(group, "aliases", "test-alias-1,test-alias-2");
  }

  void setup_keyfile_source_clone (schroot::keyfile& keyfile)
  {
    setup_keyfile_source_clone(keyfile, chroot->get_name());
  }

  void setup_keyfile_session_source_clone (schroot::keyfile&  keyfile,
                                           const std::string& group)
  {
    setup_keyfile_chroot(keyfile, group);
    keyfile.set_value(group, "name", "test-session-name");
    keyfile.set_value(group, "original-name", "test-name");
    keyfile.set_value(group, "selected-name", "test-session-name");
    keyfile.set_value(group, "aliases", "");
    keyfile.set_value(group, "description", chroot->get_description() + ' ' + _("(source chroot) (session chroot)"));
    keyfile.set_value(group, "original-name", "test-name");
    keyfile.set_value(group, "users", "user1");
    keyfile.set_value(group, "root-users", "");
    keyfile.set_value(group, "groups", "");
    keyfile.set_value(group, "root-groups", "");
  }

  void test_setup_env(const schroot::environment& observed_environment,
                      const schroot::environment& expected_environment)
  {
    ASSERT_GT(observed_environment.size(), 0);
    ASSERT_GT(expected_environment.size(), 0);

    std::set<std::string> expected;
    for (const auto& env : expected_environment)
      expected.insert(env.first);

    std::set<std::string> found;
    for (const auto& env : observed_environment)
      found.insert(env.first);

    schroot::string_list missing;
    set_difference(expected.begin(), expected.end(),
                   found.begin(), found.end(),
                   std::back_inserter(missing));
    if (!missing.empty())
      {
        std::string value;
        for (const auto& env : missing)
          {
            expected_environment.get(env, value);
            std::cout << "Missing environment: "
                      << env << "=" << value << std::endl;
          }
      }
    ASSERT_TRUE(missing.empty());

    schroot::string_list extra;
    set_difference(found.begin(), found.end(),
                   expected.begin(), expected.end(),
                   std::back_inserter(extra));
    if (!extra.empty())
      {
        std::string value;
        for (const auto&  env : extra)
          {
            observed_environment.get(env, value);
            std::cout << "Additional environment: "
                      << env << "=" << value << std::endl;
          }
      }
    ASSERT_TRUE(extra.empty());

    for (const auto& env : expected_environment)
      {
        std::string checkval;
        ASSERT_TRUE(observed_environment.get(env.first, checkval));

        if (checkval != env.second)
          std::cout << "Environment error (" << env.first << "): "
                    << checkval << " [observed] != "
                    << env.second << " [expected]"
                    << std::endl;
        ASSERT_EQ(checkval, env.second);
      }
  }

  void test_setup_env(schroot::chroot::chroot::ptr& chroot,
                      const schroot::environment&   expected_environment)
  {
    schroot::environment observed_environment;
    chroot->setup_env(observed_environment);

    ASSERT_GT(observed_environment.size(), 0);

    test_setup_env(observed_environment, expected_environment);
  }

  void test_setup_keyfile(const schroot::keyfile& observed_keyfile,
                          const std::string&      observed_group,
                          const schroot::keyfile& expected_keyfile,
                          const std::string&      expected_group)
  {
    ASSERT_GT(observed_keyfile.get_keys(observed_group).size(), 0);
    ASSERT_GT(expected_keyfile.get_keys(expected_group).size(), 0);


    schroot::string_list expected_keys =
      expected_keyfile.get_keys(expected_group);
    std::set<std::string> expected(expected_keys.begin(), expected_keys.end());

    schroot::string_list observed_keys =
      observed_keyfile.get_keys(observed_group);
    std::set<std::string> observed(observed_keys.begin(), observed_keys.end());

    schroot::string_list missing;
    set_difference(expected.begin(), expected.end(),
                   observed.begin(), observed.end(),
                   std::back_inserter(missing));
    if (!missing.empty())
      {
        std::string value;
        for (const auto& key : missing)
          {
            expected_keyfile.get_value(expected_group, key, value);
            std::cout << "Missing keys: "
                      << key << "=" << value << std::endl;
          }
      }
    ASSERT_TRUE(missing.empty());

    schroot::string_list extra;
    set_difference(observed.begin(), observed.end(),
                   expected.begin(), expected.end(),
                   std::back_inserter(extra));
    if (!extra.empty())
      {
        std::string value;
        for (const auto& key : extra)
          {
            observed_keyfile.get_value(observed_group, key, value);
            std::cout << "Additional keys: "
                      << key << "=" << value << std::endl;
          }
      }
    ASSERT_TRUE(extra.empty());

    for (const auto& key : expected_keys)
      {
        std::string expected_val;
        ASSERT_TRUE(expected_keyfile.get_value(expected_group,
                                               key, expected_val));

        std::string observed_val;
        ASSERT_TRUE(observed_keyfile.get_value(observed_group,
                                               key, observed_val));

        if (expected_val != observed_val)
          std::cout << "Keyfile error (" << key << "): "
                    << observed_val << " [observed] != "
                    << expected_val << " [expected]"
                    << std::endl;
        ASSERT_EQ(expected_val, observed_val);
      }
  }

  void test_setup_keyfile(schroot::chroot::chroot::ptr& chroot,
                          const schroot::keyfile&       expected_keyfile,
                          const std::string&            group)
  {
    schroot::keyfile keys;
    chroot->get_keyfile(keys);

    test_setup_keyfile(keys, chroot->get_name(),
                       expected_keyfile, group);
  }

  // TODO: All chroot types should check text output matches.  If
  // possible, test chroot locking functions, but this is going to be
  // tricky without having root in many cases.

};

template<class T>
void test_list(T&                         itype,
               const schroot::string_list& list,
               const schroot::string_list& (T::*getter)(void) const,
               void (T::*setter)(schroot::string_list const&))
{
  // Set items from list.
  (itype.*setter)(list);

  // Check set items exist, but make no assumptions about ordering.
  schroot::string_list set_items = (itype.*getter)();

  schroot::string_list orig_list = list;
  sort(orig_list.begin(), orig_list.end());
  sort(set_items.begin(), set_items.end());

  schroot::string_list missing;
  set_symmetric_difference(orig_list.begin(), orig_list.end(),
                           set_items.begin(), set_items.end(),
                           std::back_inserter(missing));

  if (!missing.empty())
    for (const auto& item : missing)
      {
        std::cout << "Missing list item: " << item << std::endl;
      }
  // Ensure the test is working.
  ASSERT_TRUE(missing.empty());
  ASSERT_EQ(set_items.size(), list.size());
}

#endif /* TEST_SCHROOT_CHROOT_H */

/*
 * Local Variables:
 * mode:C++
 * End:
 */
