/* sbuild-nstream - sbuild null ostream
 *
 * Copyright © 2005  Roger Leigh <rleigh@debian.org>
 *
 * serror is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * serror is distributed in the hope that it will be useful, but
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

#ifndef SBUILD_NOSTREAM_H
#define SBUILD_NOSTREAM_H

#include <streambuf>
#include <ostream>

namespace sbuild
{

  template <class cT, class traits = std::char_traits<cT> >
    class basic_nbuf: public std::basic_streambuf<cT, traits> {
      typename traits::int_type overflow(typename traits::int_type c)
      {
	return traits::not_eof(c); // indicate success
      }
    };

  template <class cT, class traits = std::char_traits<cT> >
    class basic_nostream: public std::basic_ostream<cT, traits> {
      public:
      basic_nostream():
	std::basic_ios<cT, traits>(&m_sbuf),
	std::basic_ostream<cT, traits>(&m_sbuf)
      {
	init(&m_sbuf);
      }

      private:
      basic_nbuf<cT, traits> m_sbuf;
    };

  typedef basic_nostream<char> nostream;
  typedef basic_nostream<wchar_t> wnostream;

  extern nostream cnull;

}

#endif /* SBUILD_NOSTREAM_H */

/*
 * Local Variables:
 * mode:C++
 * End:
 */