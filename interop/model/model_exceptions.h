/** Exceptions thrown by the model
 *
 *  @file
 *  @date 3/21/16
 *  @version 1.0
 *  @copyright GNU Public License.
 */
#pragma once
#include <stdexcept>
#include "interop/util/exception_specification.h"

#ifdef _MSC_VER
#pragma warning(disable:4290) // MSVC warns that it ignores the exception specification.
#endif

namespace illumina {
namespace interop {
namespace model {

    /** @defgroup model_exceptions Model Exceptions
     *
     * Exceptions that can be thrown if a problem occurs while using the model
     *
     * @ingroup interop_exceptions
     * @{
     */
    /** Exception raised if an index goes out of bounds
     */
    struct index_out_of_bounds_exception : public std::out_of_range {
        /** Constructor
         *
         *  @param mesg error message
         */
        index_out_of_bounds_exception(const std::string &mesg) : std::out_of_range(mesg) { }
    };

    /** Exception raised if the channel names are invalid
     */
    struct invalid_channel_exception : public std::runtime_error {
        /** Constructor
         *
         *  @param mesg error message
         */
        invalid_channel_exception(const std::string &mesg) : std::runtime_error(mesg) { }
    };

    /** @} */
}
}
}

