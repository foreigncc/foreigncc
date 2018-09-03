#include <common/common.h>
#include <cstdio>
#include <cstdlib>

#define BOOST_ALL_NO_LIB
#include <boost/logic/tribool.hpp>
using boost::tribool;
using boost::indeterminate;


int main(int argc, char* argv[])
{
    printf(
#if BUILD_FOREIGNCC
        "foreigncc"
#elif BUILD_FOREIGNCCD
        "foreignccd"
#elif BUILD_FOREIGNCCCTL
        "foreignccctl"
#endif
        " %d.%d.%d %s\n",
        FOREIGNCC_VERSION_MAJOR, FOREIGNCC_VERSION_MINOR, FOREIGNCC_VERSION_PATCH, FOREIGNCC_VERSION_SUFFIX);

    tribool x = indeterminate;
    printf("%d\n", indeterminate(x));

    return 0;
}
