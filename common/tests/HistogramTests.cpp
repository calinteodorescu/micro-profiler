#include <common/histogram.h>

#include <ut/assert.h>
#include <ut/test.h>

using namespace std;

namespace micro_profiler
{
	namespace tests
	{
		namespace
		{
			template <typename ScaleT>
			index_t cvt(const ScaleT &scale_, value_t value)
			{
				index_t index;

				scale_(index, value);
				return index;
			}
		}

		begin_test_suite( ScaleTests )
			test( DefaultScaleIsEmpty )
			{
				// INIT / ACT
				scale s;
				index_t index;

				// ACT / ASSERT
				assert_equal(0u, s.samples());
				assert_is_false(s(index, 0));
				assert_is_false(s(index, 0xFFFFFFF));
				assert_equal(scale(0, 0, 0), s);
			}


			test( ASingleSampledScaleAlwaysEvaluatesToZero )
			{
				// INIT / ACT
				scale s1(100, 101, 1);
				scale s2(0, 0, 1);

				// ACT / ASSERT
				assert_equal(0u, cvt(s1, 0));
				assert_equal(0u, cvt(s1, 100));
				assert_equal(0u, cvt(s1, 100000000));
				assert_equal(0u, cvt(s2, 0));
				assert_equal(0u, cvt(s2, 100000000));
			}


			test( LinearScaleGivesBoundariesForOutOfDomainValues )
			{
				// INIT
				scale s1(15901, 1000000, 3);
				scale s2(9000, 10000, 10);

				// ACT / ASSERT
				assert_equal(3u, s1.samples());
				assert_equal(0u, cvt(s1, 0));
				assert_equal(0u, cvt(s1, 15900));
				assert_equal(0u, cvt(s1, 15901));
				assert_equal(2u, cvt(s1, 1000000));
				assert_equal(2u, cvt(s1, 1000001));
				assert_equal(2u, cvt(s1, 2010001));

				assert_equal(10u, s2.samples());
				assert_equal(0u, cvt(s2, 0));
				assert_equal(0u, cvt(s2, 8000));
				assert_equal(0u, cvt(s2, 9000));
				assert_equal(9u, cvt(s2, 10000));
				assert_equal(9u, cvt(s2, 10001));
				assert_equal(9u, cvt(s2, 2010001));
			}


			test( LinearScaleProvidesIndexForAValueWithBoundariesAtTheMiddleOfRange )
			{
				// INIT
				scale s1(1000, 3000, 3);
				scale s2(10, 710, 10);

				// ACT / ASSERT
				assert_equal(0u, cvt(s1, 1499));
				assert_equal(1u, cvt(s1, 1500));
				assert_equal(1u, cvt(s1, 2499));
				assert_equal(2u, cvt(s1, 2500));

				assert_equal(0u, cvt(s2, 49));
				assert_equal(1u, cvt(s2, 50));
				assert_equal(3u, cvt(s2, 283));
				assert_equal(4u, cvt(s2, 284));
				assert_equal(8u, cvt(s2, 671));
				assert_equal(9u, cvt(s2, 673));
			}


			test( ScaleIsEquallyComparable )
			{
				// INIT
				scale s1(1000, 3000, 3);
				scale s11(1000, 3000, 3);
				scale s2(10, 710, 10);
				scale s21(10, 710, 10);
				scale s3(1000, 3000, 90);
				scale s4(1000, 3050, 3);
				scale s5(1001, 3000, 3);

				// ACT / ASSERT
				assert_is_true(s1 == s1);
				assert_is_true(s1 == s11);
				assert_is_false(s1 == s2);
				assert_is_false(s1 == s3);
				assert_is_false(s1 == s4);
				assert_is_false(s1 == s5);
				assert_is_true(s2 == s2);
				assert_is_true(s2 == s21);

				assert_is_false(s1 != s1);
				assert_is_false(s1 != s11);
				assert_is_true(s1 != s2);
				assert_is_true(s1 != s3);
				assert_is_true(s1 != s4);
				assert_is_true(s1 != s5);
				assert_is_false(s2 != s2);
				assert_is_false(s2 != s21);
			}
		end_test_suite


		begin_test_suite( HistogramTests )
			test( HistogramIsResizedOnSettingScale )
			{
				// INIT
				histogram h;

				// INIT / ACT
				h.set_scale(scale(0, 90, 10));

				// ACT / ASSERT
				assert_equal(10u, h.size());
				assert_equal(10, distance(h.begin(), h.end()));
				assert_equal(scale(0, 90, 10), h.get_scale());

				// INIT / ACT
				h.set_scale(scale(3, 91, 17));

				// ACT / ASSERT
				assert_equal(17u, h.size());
				assert_equal(17, distance(h.begin(), h.end()));
				assert_equal(scale(3, 91, 17), h.get_scale());
			}


			test( CountsAreIncrementedAtExpectedBins )
			{
				// INIT
				histogram h;

				h.set_scale(scale(0, 90, 10));

				// ACT
				h.add(4);
				h.add(3);
				h.add(5);
				h.add(85);

				// ASSERT
				unsigned reference1[] = {	2, 1, 0, 0, 0, 0, 0, 0, 0, 1,	};

				assert_equal(reference1, h);

				// ACT
				h.add(50);
				h.add(51);
				h.add(81, 3);

				// ASSERT
				unsigned reference2[] = {	2, 1, 0, 0, 0, 2, 0, 0, 3, 1,	};

				assert_equal(reference2, h);
			}


			test( HistogramIsResetAtRescale )
			{
				// INIT
				histogram h;
				h.set_scale(scale(0, 900, 5));

				h.add(10);
				h.add(11);
				h.add(9);
				h.add(800);

				// ACT
				h.set_scale(scale(0, 90, 7));

				// ASSERT
				unsigned reference[] = {	0, 0, 0, 0, 0, 0, 0,	};

				assert_equal(reference, h);
			}


			test( HistogramIsResetOnAddingDifferentlyScaledRhs )
			{
				// INIT
				histogram h, addition;

				h.set_scale(scale(0, 900, 5));
				addition.set_scale(scale(10, 900, 5));

				h.add(10), h.add(11), h.add(9), h.add(800);

				// ACT / ASSERT
				assert_equal(&h, &(h += addition));

				// ASSERT
				unsigned reference1[] = {	0, 0, 0, 0, 0,	};

				assert_equal(reference1, h);
				assert_equal(scale(10, 900, 5), h.get_scale());

				// INIT
				h.add(190);
				addition.set_scale(scale(10, 900, 12));
				addition.add(50);
				addition.add(750);

				// ACT
				h += addition;

				// ASSERT
				unsigned reference2[] = {	1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0,	};

				assert_equal(reference2, h);
				assert_equal(scale(10, 900, 12), h.get_scale());
			}


			test( HistogramIsResetOnInterpolatingWithADifferentlyScaledOne )
			{
				// INIT
				histogram h, addition;

				h.set_scale(scale(0, 900, 5));
				addition.set_scale(scale(10, 900, 5));

				h.add(10), h.add(11), h.add(9), h.add(800);

				// ACT
				interpolate(h, addition, 1.0f);

				// ASSERT
				unsigned reference1[] = {	0, 0, 0, 0, 0,	};

				assert_equal(reference1, h);
				assert_equal(scale(10, 900, 5), h.get_scale());

				// INIT
				h.add(190);
				addition.set_scale(scale(10, 900, 12));
				addition.add(50);
				addition.add(750);

				// ACT
				interpolate(h, addition, 1.0f);

				// ASSERT
				unsigned reference2[] = {	1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0,	};

				assert_equal(reference2, h);
				assert_equal(scale(10, 900, 12), h.get_scale());
			}


			test( HistogramIsAddedToTheEquallyScaledValue )
			{
				// INIT
				histogram h, addition;

				h.set_scale(scale(0, 10, 11));
				addition.set_scale(scale(0, 10, 11));

				h.add(2);
				h.add(7);
				addition.add(1, 2);
				addition.add(7, 3);
				addition.add(8);

				// ACT
				h += addition;

				// ASSERT
				unsigned reference[] = {	0, 2, 1, 0, 0, 0, 0, 4, 1, 0, 0	};

				assert_equal(reference, h);
			}


			test( HistogramValuesAreInterpolatedAsRequested )
			{
				// INIT
				histogram h, addition;

				h.set_scale(scale(0, 5, 6));
				addition.set_scale(scale(0, 5, 6));

				addition.add(0, 100);
				addition.add(2, 51);
				addition.add(3, 117);

				// ACT
				interpolate(h, addition, 7.0f / 256);

				// ASSERT
				unsigned reference1[] = {	2, 0, 1, 3, 0, 0,	};

				assert_equal(reference1, h);

				// INIT
				const auto addition2 = h;

				// ACT
				interpolate(h, addition, 1.0f);

				// ASSERT
				unsigned reference2[] = {	100, 0, 51, 117, 0, 0,	};

				assert_equal(reference2, h);

				// ACT
				interpolate(h, addition2, 0.5f);

				// ASSERT
				unsigned reference3[] = {	51, 0, 26, 60, 0, 0,	};

				assert_equal(reference3, h);
			}


			test( DefaultConstructedHistogramEmptyButAcceptsValues )
			{
				// INIT / ACT
				histogram h;

				// ACT / ASSERT
				assert_equal(scale(), h.get_scale());
				assert_equal(0u, h.size());

				// ACT / ASSERT
				h.add(0);
				h.add(1000000);
			}


			test( ResettingHistogramPreservesScaleAndSizeButSetsValuesToZeroes )
			{
				// INIT / ACT
				histogram h;

				h.set_scale(scale(0, 9, 10));
				h.add(2, 9);
				h.add(7, 3);

				// ACT
				h.reset();

				// ASSERT
				unsigned reference[] = {	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	};

				assert_equal(reference, h);
			}
		end_test_suite
	}
}
