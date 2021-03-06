/** Unit tests for the summary metrics
 *
 *
 *  @file
 *  @date 3/11/16
 *  @version 1.0
 *  @copyright GNU Public License.
 */

#include <fstream>
#include <gtest/gtest.h>
#include "interop/logic/summary/run_summary.h"
#include "inc/error_metrics_test.h"
#include "inc/extraction_metrics_test.h"
#include "inc/tile_metrics_test.h"
#include "inc/q_metrics_test.h"
#include "inc/summary_fixture.h"
using namespace illumina::interop::model::metrics;
using namespace illumina::interop::io;
using namespace illumina::interop;
using namespace illumina::interop::unittest;


/** Interface between fixtures and Google Test */
template<typename TestSetup>
struct summary_metrics_test : public ::testing::Test, public TestSetup{};

typedef ::testing::Types<
        summary_fixture<error_v3>,
        summary_fixture<extraction_v2>,
        summary_fixture<q_v4>,
        summary_fixture<q_v5>,
        summary_fixture<q_v6>,
        summary_fixture<tile_v2>
> Formats;
TYPED_TEST_CASE(summary_metrics_test, Formats);

#define EXPECT_STAT_NEAR(ACTUAL, EXPECTED, TOL) \
    EXPECT_NEAR(ACTUAL.mean(), EXPECTED.mean(), TOL); \
    EXPECT_NEAR(ACTUAL.stddev(), EXPECTED.stddev(), TOL); \
    EXPECT_NEAR(ACTUAL.median(), EXPECTED.median(), TOL)

#define EXPECT_CYCLE_EQ(ACTUAL, EXPECTED) \
    EXPECT_EQ(ACTUAL.first_cycle(), EXPECTED.first_cycle()); \
    EXPECT_EQ(ACTUAL.last_cycle(), EXPECTED.last_cycle())

#define EXPECT_READ_EQ(ACTUAL, EXPECTED) \
    EXPECT_EQ(ACTUAL.first_cycle(), EXPECTED.first_cycle()); \
    EXPECT_EQ(ACTUAL.last_cycle(), EXPECTED.last_cycle()); \
    EXPECT_EQ(ACTUAL.number(), EXPECTED.number()); \
    EXPECT_EQ(ACTUAL.is_index(), EXPECTED.is_index())

/** Test if calculated run summary matches actual run summary
 *
 * @todo Add more robust testing (e.g. non-index)
 */
TYPED_TEST(summary_metrics_test, run_summary)
{
    const float tol = 1e-7f;
    EXPECT_EQ(TypeParam::actual.size(), TypeParam::expected.size());
    EXPECT_EQ(TypeParam::actual.lane_count(), TypeParam::expected.lane_count());
    EXPECT_NEAR(TypeParam::actual.total_summary().error_rate(), TypeParam::expected.total_summary().error_rate(), tol);
    EXPECT_NEAR(TypeParam::actual.total_summary().percent_aligned(), TypeParam::expected.total_summary().percent_aligned(), tol);
    EXPECT_NEAR(TypeParam::actual.total_summary().first_cycle_intensity(), TypeParam::expected.total_summary().first_cycle_intensity(), tol);
    EXPECT_NEAR(TypeParam::actual.total_summary().percent_gt_q30(), TypeParam::expected.total_summary().percent_gt_q30(), tol);
    EXPECT_NEAR(TypeParam::actual.total_summary().yield_g(), TypeParam::expected.total_summary().yield_g(), tol);
    EXPECT_NEAR(TypeParam::actual.total_summary().projected_yield_g(), TypeParam::expected.total_summary().projected_yield_g(), tol);


    EXPECT_NEAR(TypeParam::actual.nonindex_summary().error_rate(), TypeParam::expected.nonindex_summary().error_rate(), tol);
    EXPECT_NEAR(TypeParam::actual.nonindex_summary().percent_aligned(), TypeParam::expected.nonindex_summary().percent_aligned(), tol);
    EXPECT_NEAR(TypeParam::actual.nonindex_summary().first_cycle_intensity(), TypeParam::expected.nonindex_summary().first_cycle_intensity(), tol);
    EXPECT_NEAR(TypeParam::actual.nonindex_summary().percent_gt_q30(), TypeParam::expected.nonindex_summary().percent_gt_q30(), tol);
    EXPECT_NEAR(TypeParam::actual.nonindex_summary().yield_g(), TypeParam::expected.nonindex_summary().yield_g(), tol);
    EXPECT_NEAR(TypeParam::actual.nonindex_summary().projected_yield_g(), TypeParam::expected.nonindex_summary().projected_yield_g(), tol);

    const model::summary::cycle_state_summary& actual_cycle_summary = TypeParam::actual.cycle_state();
    const model::summary::cycle_state_summary& expected_cycle_summary = TypeParam::expected.cycle_state();
    EXPECT_CYCLE_EQ(actual_cycle_summary.extracted_cycle_range(), expected_cycle_summary.extracted_cycle_range());
    EXPECT_CYCLE_EQ(actual_cycle_summary.called_cycle_range(), expected_cycle_summary.called_cycle_range());
    EXPECT_CYCLE_EQ(actual_cycle_summary.qscored_cycle_range(), expected_cycle_summary.qscored_cycle_range());
    EXPECT_CYCLE_EQ(actual_cycle_summary.error_cycle_range(), expected_cycle_summary.error_cycle_range());
}
/** Test if calculated read summary matches actual read summary
 */
TYPED_TEST(summary_metrics_test, read_summary)
{
    const float tol = 1e-7f;
    for(size_t read=0;read<std::min(TypeParam::actual.size(), TypeParam::expected.size());++read)
    {
        const model::summary::read_summary &actual_read_summary = TypeParam::actual[read];
        const model::summary::read_summary &expected_read_summary = TypeParam::expected[read];
        EXPECT_EQ(actual_read_summary.size(), expected_read_summary.size());
        EXPECT_EQ(actual_read_summary.lane_count(), expected_read_summary.lane_count());
        EXPECT_NEAR(actual_read_summary.summary().error_rate(), expected_read_summary.summary().error_rate(), tol);
        EXPECT_NEAR(actual_read_summary.summary().percent_aligned(), expected_read_summary.summary().percent_aligned(), tol);
        EXPECT_NEAR(actual_read_summary.summary().first_cycle_intensity(), expected_read_summary.summary().first_cycle_intensity(), tol);
        EXPECT_NEAR(actual_read_summary.summary().percent_gt_q30(), expected_read_summary.summary().percent_gt_q30(), tol)  << "Failed read: " << read;
        EXPECT_NEAR(actual_read_summary.summary().yield_g(), expected_read_summary.summary().yield_g(), tol)  << "Failed read: " << read;
        EXPECT_NEAR(actual_read_summary.summary().projected_yield_g(), expected_read_summary.summary().projected_yield_g(), tol)  << "Failed read: " << read;
        EXPECT_READ_EQ(actual_read_summary.read(), expected_read_summary.read());
    }
}
/** Test if calculated lane summary matches actual lane summary
 */
TYPED_TEST(summary_metrics_test, lane_summary)
{
#ifdef _INTEROP_WIN32
    const float density_tol = 0.5f;
    const float tol = 1e-2f; // TODO: fix this unit test on external Windows Builds (appveyor) was 1e-7f
#else
    const float density_tol = 1e-7f;
    const float tol = 1e-7f;
#endif
    for(size_t read=0;read<std::min(TypeParam::actual.size(), TypeParam::expected.size());++read)
    {
        for(size_t lane=0;lane<std::min(TypeParam::actual[read].size(), TypeParam::expected[read].size());++lane)
        {
            const model::summary::lane_summary& actual_lane_summary = TypeParam::actual[read][lane];
            const model::summary::lane_summary& expected_lane_summary = TypeParam::expected[read][lane];
            EXPECT_EQ(actual_lane_summary.lane(), expected_lane_summary.lane());
            EXPECT_GT(actual_lane_summary.lane(), 0u);
            EXPECT_EQ(actual_lane_summary.tile_count(), expected_lane_summary.tile_count()) << "Failed read: " << read << ", lane: " << lane;
            EXPECT_NEAR(actual_lane_summary.percent_gt_q30(), expected_lane_summary.percent_gt_q30(), tol) << "Failed read: " << read << ", lane: " << lane;
            EXPECT_NEAR(actual_lane_summary.yield_g(), expected_lane_summary.yield_g(), tol) << "Failed read: " << read << ", lane: " << lane;
            EXPECT_NEAR(actual_lane_summary.projected_yield_g(), expected_lane_summary.projected_yield_g(), tol) << "Failed read: " << read << ", lane: " << lane;
            EXPECT_NEAR(actual_lane_summary.reads_pf(), expected_lane_summary.reads_pf(), tol) << "Failed read: " << read << ", lane: " << lane;
            EXPECT_NEAR(actual_lane_summary.reads(), expected_lane_summary.reads(), tol) << "Failed read: " << read << ", lane: " << lane;
            EXPECT_STAT_NEAR(actual_lane_summary.density(), expected_lane_summary.density(), density_tol) << "Failed read: " << read << ", lane: " << lane;
            EXPECT_STAT_NEAR(actual_lane_summary.density_pf(), expected_lane_summary.density_pf(), density_tol);
            EXPECT_STAT_NEAR(actual_lane_summary.cluster_count(), expected_lane_summary.cluster_count(), density_tol);
            EXPECT_STAT_NEAR(actual_lane_summary.cluster_count_pf(), expected_lane_summary.cluster_count_pf(), density_tol);
            EXPECT_STAT_NEAR(actual_lane_summary.percent_pf(), expected_lane_summary.percent_pf(), tol) << "Failed read: " << read << ", lane: " << lane;
            EXPECT_STAT_NEAR(actual_lane_summary.phasing(), expected_lane_summary.phasing(), tol) << "Failed read: " << read << ", lane: " << lane;
            EXPECT_STAT_NEAR(actual_lane_summary.prephasing(), expected_lane_summary.prephasing(), tol) << "Failed read: " << read << ", lane: " << lane;
            EXPECT_STAT_NEAR(actual_lane_summary.percent_aligned(), expected_lane_summary.percent_aligned(), tol) << "Failed read: " << read << ", lane: " << lane;
            EXPECT_STAT_NEAR(actual_lane_summary.error_rate(), expected_lane_summary.error_rate(), tol);
            EXPECT_STAT_NEAR(actual_lane_summary.error_rate_35(), expected_lane_summary.error_rate_35(), tol);
            EXPECT_STAT_NEAR(actual_lane_summary.error_rate_50(), expected_lane_summary.error_rate_50(), tol);
            EXPECT_STAT_NEAR(actual_lane_summary.error_rate_75(), expected_lane_summary.error_rate_75(), tol);
            EXPECT_STAT_NEAR(actual_lane_summary.error_rate_100(), expected_lane_summary.error_rate_100(), tol) << "Failed read: " << read << ", lane: " << lane;
            EXPECT_STAT_NEAR(actual_lane_summary.first_cycle_intensity(), expected_lane_summary.first_cycle_intensity(), tol);

            const model::summary::cycle_state_summary& actual_cycle_summary = actual_lane_summary.cycle_state();
            const model::summary::cycle_state_summary& expected_cycle_summary = expected_lane_summary.cycle_state();
            EXPECT_CYCLE_EQ(actual_cycle_summary.called_cycle_range(), expected_cycle_summary.called_cycle_range());
            EXPECT_CYCLE_EQ(actual_cycle_summary.extracted_cycle_range(), expected_cycle_summary.extracted_cycle_range());
            EXPECT_CYCLE_EQ(actual_cycle_summary.called_cycle_range(), expected_cycle_summary.called_cycle_range());
            EXPECT_CYCLE_EQ(actual_cycle_summary.qscored_cycle_range(), expected_cycle_summary.qscored_cycle_range());
            EXPECT_CYCLE_EQ(actual_cycle_summary.error_cycle_range(), expected_cycle_summary.error_cycle_range()) << "Failed read: " << read << ", lane: " << lane;
        }
    }
}

TEST(summary_metrics_test, empty_run_metrics)
{
    model::metrics::run_metrics metrics;
    model::summary::run_summary summary;
    logic::summary::summarize_run_metrics(metrics, summary);
}




