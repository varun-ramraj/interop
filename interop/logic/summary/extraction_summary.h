/** Summary logic for extraction metrics
 *
 *  @file
 *  @date  3/5/16
 *  @version 1.0
 *  @copyright GNU Public License.
 */
#pragma once
#include <vector>
#include "interop/model/model_exceptions.h"
#include "interop/logic/summary/summary_statistics.h"
#include "interop/model/metrics/extraction_metric.h"
#include "interop/logic/summary/map_cycle_to_read.h"
#include "interop/model/summary/run_summary.h"


namespace illumina
{
namespace interop
{
namespace logic
{
namespace summary
{
    /** Summarize a collection extraction metrics
     *
     * @sa model::summary::lane_summary::first_cycle_intensity
     * @sa model::summary::read_summary::first_cycle_intensity
     * @sa model::summary::run_summary::first_cycle_intensity
     *
     *
     * @param beg iterator to start of a collection of extraction metrics
     * @param end iterator to end of a collection of extraction metrics
     * @param cycle_to_read map cycle to the read number and cycle within read number
     * @param channel channel to use for intensity reporting
     * @param run destination run summary
     */
    template<typename I>
    void summarize_extraction_metrics(I beg,
                                      I end,
                                      const read_cycle_vector_t& cycle_to_read,
                                      const size_t channel,
                                      model::summary::run_summary &run) throw( model::index_out_of_bounds_exception )
    {
        typedef typename model::metrics::extraction_metric::ushort_t ushort_t;
        typedef summary_by_lane_read<ushort_t> summary_by_lane_read_t;
        if(beg == end) return;
        if(run.size()==0)return;
        summary_by_lane_read_t temp(run, std::distance(beg, end));

        for(;beg != end;++beg)
        {
            const size_t read = cycle_to_read[beg->cycle()-1].number-1;
            if(cycle_to_read[beg->cycle()-1].cycle_within_read > 1) continue;
            INTEROP_ASSERT(read < temp.read_count());
            const size_t lane = beg->lane()-1;
            if(lane >= temp.lane_count()) throw model::index_out_of_bounds_exception("Lane exceeds lane count in RunInfo.xml");
            temp(read, lane).push_back(beg->max_intensity(channel));
        }

        float first_cycle_intensity = 0;
        size_t total = 0;
        float first_cycle_intensity_nonindex = 0;
        size_t total_nonindex = 0;
        for(size_t read=0;read<run.size();++read)
        {
            INTEROP_ASSERT(read < temp.read_count());
            INTEROP_ASSERT(read < run.size());
            float first_cycle_intensity_by_read = 0;
            size_t total_by_read = 0;
            for(size_t lane=0;lane<run[read].size();++lane)
            {
                INTEROP_ASSERT(lane < temp.lane_count());
                INTEROP_ASSERT(lane < run[read].size());
                summarize(temp(read,lane).begin(), temp(read,lane).end(), run[read][lane].first_cycle_intensity());
                first_cycle_intensity_by_read += std::accumulate(temp(read,lane).begin(),
                                                      temp(read,lane).end(),
                                                      size_t(0));
                total_by_read += temp(read,lane).size();
            }
            run[read].summary().first_cycle_intensity(divide(first_cycle_intensity_by_read, static_cast<float>(total_by_read)));
            first_cycle_intensity += first_cycle_intensity_by_read;
            total += total_by_read;
            if(!run[read].read().is_index())
            {
                first_cycle_intensity_nonindex += first_cycle_intensity_by_read;
                total_nonindex += total_by_read;
            }
        }
        run.nonindex_summary().first_cycle_intensity(divide(first_cycle_intensity_nonindex, static_cast<float>(total_nonindex)));
        run.total_summary().first_cycle_intensity(divide(first_cycle_intensity, static_cast<float>(total)));
    }

}
}
}
}
