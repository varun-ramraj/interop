/** @page apps Applications
 *
 * @brief Developer application to sample records and write bytes as text
 *
 * This developer application is to help create unit tests. The dumpbin application reads in an InterOp files, selects
 * a subset of records (default 3) and writes the binary files as a set command separated bytes to the console as
 * text.
 *
 * Running the Program
 * -------------------
 *
 * The program runs as follows:
 *
 *      $ dumpbin 140131_1287_0851_A01n401drr
 *
 * In this sample, 140131_1287_0851_A01n401drr is a run folder and the summary is written to the standard output.
 *
 * # Version: v1.0.4-70-g9bcfb5a-dirty
 * Error
 * 3,30,3,0,63,8,1,0,120,-26,27,63,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,0,63,8,2,0,113,-11,-104,62,0,0,0,0,0,0,0,0,
 * 0,0,0,0,0,0,0,0,0,0,0,0,3,0,63,8,3,0,98,19,-109,62,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
 * Extraction
 * 2,38,3,0,63,8,1,0,40,118,9,64,-4,-65,7,64,0,0,0,0,0,0,0,0,109,3,6,2,0,0,0,0,-128,-11,-59,24,-68,99,-45,-120,3,0,64,8,
 * 1,0,68,125,10,64,-14,80,5,64,0,0,0,0,0,0,0,0,94,3,-4,1,0,0,0,0,-128,-11,-59,24,-68,99,-45,-120,3,0,65,8,1,0,-86,20,
 * 10,64,78,14,6,64,0,0,0,0,0,0,0,0,119,3,7,2,0,0,0,0,-128,-11,-59,24,-68,99,-45,-120
 * Tile
 * 2,10,3,0,63,8,100,0,-47,8,105,74,3,0,63,8,101,0,-117,-34,32,74,3,0,63,8,102,0,64,-3,-43,74,3,0,63,8,103,0,-46,-72,
 * -109,74,3,0,63,8,-55,0,0,0,0,0,3,0,63,8,-56,0,0,0,0,0,3,0,63,8,44,1,33,101,33,64,3,0,64,8,100,0,-47,8,105,74,3,0,64,
 * 8,101,0,87,-123,37,74,3,0,64,8,102,0,64,-3,-43,74,3,0,64,8,103,0,76,-2,-105,74,3,0,64,8,-55,0,0,0,0,0,3,0,64,8,-56,
 * 0,0,0,0,0,3,0,64,8,44,1,-70,-121,34,64,3,0,65,8,100,0,-47,8,105,74,3,0,65,8,101,0,32,-23,35,74,3,0,65,8,102,0,64,-3,
 * -43,74,3,0,65,8,103,0,-58,-125,-106,74,3,0,65,8,-55,0,0,0,0,0,3,0,65,8,-56,0,0,0,0,0,3,0,65,8,44,1,-71,9,34,64
 */

#include <iostream>
#include <iomanip>
#include "interop/io/metric_file_stream.h"
#include "interop/model/run_metrics.h"
#include "interop/version.h"

using namespace illumina::interop::model::metrics;
using namespace illumina::interop;

/** Exit codes that can be produced by the application
 */
enum exit_codes
{
    /** The program exited cleanly, 0 */
    SUCCESS,
    /** Invalid arguments were given to the application*/
    INVALID_ARGUMENTS,
    /** Empty InterOp directory*/
    NO_INTEROPS_FOUND,
    /** InterOp file has a bad format */
    BAD_FORMAT,
    /** Unknown error has occurred*/
    UNEXPECTED_EXCEPTION,
    /** InterOp file has not records */
    EMPTY_INTEROP
};

/** Call back functor for writing binary metric data as a string
 */
struct metric_writer
{
    /** Constructor
     *
     * @param out output stream
     * @param max_line number of characters before wrapping
     */
    metric_writer(std::ostream& out, const size_t max_line=80) : m_out(out), m_max_line(max_line){}
    /** Function operator overload to write data
     *
     * @param metrics set of metrics
     */
    template<class MetricSet>
    void operator()(const MetricSet& metrics)const
    {
        if(metrics.size()==0) return;
        std::ostringstream fout;
        io::write_metrics(fout, metrics, metrics.version());
        m_out << metrics.prefix() << std::endl;
        write_bytes_as_string(m_out, fout.str(), m_max_line);
    }
private:
    static void write_bytes_as_string(std::ostream& out, const std::string& buffer, const size_t k_max_line)
    {
        std::string::const_iterator sit = buffer.begin();
        if (sit == buffer.end()) return;

        out << ::int16_t(*sit);
        size_t char_count = length_of(::int16_t(*sit));
        for (++sit; sit != buffer.end(); ++sit, ++char_count) {
            const ::int16_t val = ::int16_t(*sit);
            char_count += 1 + length_of(val);
            if (char_count > k_max_line) {
                out << "\n";
                char_count = 1 + length_of(val);
            }
            out << "," << val;
        }
        out << std::endl;

    }
    static size_t length_of(const ::int16_t val)
    {
        if(val <= -100) return 4;
        if(val >= 100) return 3;
        if(val <= -10) return 3;
        if(val >= 10) return 2;
        if(val < 0) return 2;
        return 1;
    }
private:
    metric_writer& operator=(const metric_writer&){return *this;}
private:
    std::ostream& m_out;
    size_t m_max_line;

};
/** Copy of subset of metrics
 */
struct subset_copier
{
    /** Constructor
     *
     * @param run run metrics
     * @param total number to subsample
     */
    subset_copier(run_metrics& run, const size_t total) : m_run(run), m_total(total){}

    /** Function operator overload to collect a subset of metrics
     *
     * @param metrics set of metrics
     */
    template<class MetricSet>
    void operator()(const MetricSet& metrics)const
    {
        m_run.get<MetricSet>() = MetricSet(metrics, metrics.version());
        const size_t total = std::min(m_total, metrics.size());
        for(size_t i=0;i<total;++i)
            m_run.get<MetricSet>().insert(metrics.metrics()[i]);
    }
private:
    subset_copier& operator=(const subset_copier&){return *this;}
private:
    run_metrics& m_run;
    size_t m_total;
};

int main(int argc, char** argv)
{
    if(argc == 0)
    {
        std::cerr << "No arguments specified!" << std::endl;
        //print_help(std::cout);
        return INVALID_ARGUMENTS;
    }
    const size_t subset_count=3;

    metric_writer write_metrics(std::cout);
    std::cout << "# Version: " << INTEROP_VERSION << std::endl;

    for(int i=1;i<argc;i++)
    {
        run_metrics run;
        run_metrics subset;
        try
        {
            std::cout << "# Run Folder: " << io::basename(argv[i]) << std::endl;
            run.read_metrics(argv[i]);
        }
        catch(const io::bad_format_exception& ex)
        {
            std::cerr << ex.what() << std::endl;
            return BAD_FORMAT;
        }
        catch(const std::exception& ex)
        {
            std::cerr << ex.what() << std::endl;
            return UNEXPECTED_EXCEPTION;
        }
        if(run.empty())
        {
            std::cerr << "No InterOp files found" << std::endl;
            return EMPTY_INTEROP;
        }

        subset_copier copy_subset(subset, subset_count);
        run.metrics_callback(copy_subset);
        subset.metrics_callback(write_metrics);
        std::cout << "\n" << std::endl;

    }
    return SUCCESS;
}


