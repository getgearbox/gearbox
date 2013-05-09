// Copyright (c) 2012, Yahoo! Inc.  All rights reserved.
// Copyrights licensed under the New BSD License. See the accompanying LICENSE file for terms.

#include <tap/trivial.h>

#include <gearbox/core/util.h>
using namespace Gearbox;

int main()
{
    chdir(TESTDIR);
    TEST_START(14);

    std::vector<std::string> words;

    words.clear();
    IS( shellquote_list(words), "" );

    words.clear();
    words.push_back("single-worded");
    IS( shellquote_list(words), "single-worded" );

    words.clear();
    words.push_back("two words");
    IS( shellquote_list(words), "'two words'" );

    words.clear();
    words.push_back("separate");
    words.push_back("words");
    IS( shellquote_list(words), "separate words" );

    words.clear();
    words.push_back("ls");
    words.push_back("-ld");
    words.push_back("/tmp");
    IS( shellquote_list(words), "ls -ld /tmp" );

    words.clear();
    words.push_back("ls");
    words.push_back("*");
    IS( shellquote_list(words), "ls '*'" );

    words.clear();
    words.push_back("\"quotes\"");
    IS( shellquote_list(words), "'\"quotes\"'" );

    words.clear();
    words.push_back("\"");
    IS( shellquote_list(words), "'\"'" );

    words.clear();
    words.push_back("-m");
    words.push_back("");
    IS( shellquote_list(words), "-m ''" );

    words.clear();
    words.push_back("'");
    IS( shellquote_list(words), "''\"'\"''" );

    words.clear();
    words.push_back("'and");
    IS( shellquote_list(words), "''\"'\"'and'" );

    words.clear();
    words.push_back("here's");
    IS( shellquote_list(words), "'here'\"'\"'s'" );

    words.clear();
    words.push_back("some'thing'");
    IS( shellquote_list(words), "'some'\"'\"'thing'\"'\"''" );

    words.clear();
    words.push_back("some'thing'odd");
    IS( shellquote_list(words), "'some'\"'\"'thing'\"'\"'odd'" );

    TEST_END;
}

/* Style Settings:
 *
 * Local Variables:
 *   mode: C++
 *   indent-tabs-mode: nil
 *   c-basic-offset: 4
 *   c-set-style: "knr"
 * End:
 * vim:set sw=4 ts=4 sts=4 cindent expandtab cinoptions=(0:
 */
