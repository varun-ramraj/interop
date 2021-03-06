using System;
using NUnit.Framework;
using System.IO;

namespace illumina.interop.csharp.unittest
{
	/// <summary>
	/// Confirm that the index metrics InterOp works properly in C#
	/// </summary>
	[TestFixture]
	public class IndexMetricsTestV3
	{
		const int Version = 1;
		index_metrics expected_metric_set;
		index_metrics actual_metric_set = new index_metrics();
		vector_index_metrics expected_metrics = new vector_index_metrics();
		vector_index_metrics actual_metrics;
		byte[] expected_binary_data;

		/// <summary>
		/// Sets up the expected and actual metric sets for the test
		/// </summary>
		[SetUp]
		protected void SetUp()
		{

		    index_info_vector indices1 = new index_info_vector();
		    indices1.Add(new index_info("ATCACGAC-AAGGTTCA", "1", "TSCAIndexes", 4570));
			expected_metrics.Add(new index_metric(1, 12106, 3, indices1));
		    index_info_vector indices2 = new index_info_vector();
		    indices2.Add(new index_info("ACAGTGGT-AAGGTTCA", "2", "TSCAIndexes", 4477));
			expected_metrics.Add(new index_metric(2, 12106, 3, indices2));
		    index_info_vector indices3 = new index_info_vector();
		    indices3.Add(new index_info("CAGATCCA-AAGGTTCA", "3", "TSCAIndexes", 4578));
			expected_metrics.Add(new index_metric(3, 12106, 3, indices3));

			int[] tmp = new int[]{1
              ,1,0,74,47,3,0
              ,17,0,65,84,67,65,67,71,65,67,45,65,65,71,71,84,84,67,65,218,17,0,0,1,0,49,11,0,84,83,67,65,73,110
              ,100,101,120,101,115
              ,2,0,74,47,3,0 // changed to lane 2
              ,17,0,65,67,65,71,84,71,71,84,45,65,65,71,71,84,84,67,65,125,17,0,0,1,0,50,11,0,84,83,67,65,73,110
              ,100,101,120,101,115
              ,3,0,74,47,3,0 // changed to lane 3
              ,17,0,67,65,71,65,84,67,67,65,45,65,65,71,71,84,84,67,65,226,17,0,0,1,0,51,11,0,84,83,67,65,73,110
              ,100,101,120,101,115
			};
			expected_binary_data = new byte[tmp.Length];
			for(int i=0;i<expected_binary_data.Length;i++) expected_binary_data[i] = (byte)tmp[i];
			expected_metric_set = new index_metrics(expected_metrics, Version);
			c_csharp_interop.read_interop_from_buffer(expected_binary_data, (uint)expected_binary_data.Length, actual_metric_set);
			actual_metrics = actual_metric_set.metrics();
		}

		/// <summary>
		/// Confirms that the data was properly parsed and matches the expected model.
		/// This test also confirms that the binding gives the expected results.
		/// </summary>
		[Test]
		public void TestHardCodedBinaryData()
		{
			Assert.AreEqual(expected_metric_set.version(),  actual_metric_set.version());
			Assert.AreEqual(expected_metric_set.size(),  actual_metric_set.size());

			for(int i=0;i<Math.Min(expected_metrics.Count, actual_metrics.Count);i++)
			{
				Assert.AreEqual(expected_metrics[i].lane(), actual_metrics[i].lane());
				Assert.AreEqual(expected_metrics[i].tile(), actual_metrics[i].tile());
				Assert.AreEqual(expected_metrics[i].read(), actual_metrics[i].read());
				Assert.AreEqual(expected_metrics[i].size(), actual_metrics[i].size());
				for(uint j=0;j<Math.Min(expected_metrics[i].size(),actual_metrics[i].size());j++)
				{
					Assert.AreEqual(expected_metrics[i].indices(j).index_seq(), actual_metrics[i].indices(j).index_seq());
					Assert.AreEqual(expected_metrics[i].indices(j).sample_id(), actual_metrics[i].indices(j).sample_id());
					Assert.AreEqual(expected_metrics[i].indices(j).sample_proj(), actual_metrics[i].indices(j).sample_proj());
					Assert.AreEqual(expected_metrics[i].indices(j).count(), actual_metrics[i].indices(j).count());
				}
			}
		}
	}
}