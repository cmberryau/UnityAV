#pragma once

// can't be included as part of stdafx, causes problems in avlib code
#include <BasicUsageEnvironment.hh>

namespace UnityAV
{
    namespace Media
    {
        /**
         * \brief Responsible for deleting a basic usage environment instance
         */
        struct BasicUsageEnvrionmentDeleter
        {
            void operator()(BasicUsageEnvironment* env)
            {
                env->reclaim();
            }
        };
    }
}
