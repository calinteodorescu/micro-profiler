#include <frontend/symbol_resolver.h>

#include "helpers.h"

#include <frontend/serialization.h>

#include <common/protocol.h>
#include <strmd/deserializer.h>
#include <strmd/serializer.h>
#include <test-helpers/helpers.h>
#include <ut/assert.h>
#include <ut/test.h>

using namespace std;

namespace micro_profiler
{
	namespace tests
	{
		begin_test_suite( SymbolResolverTests )
			vector<unsigned> _requested;

			function<void (unsigned persistent_id)> get_requestor()
			{
				return [this] (unsigned persistent_id) {
					_requested.push_back(persistent_id);
				};
			}

			test( EmptyNameIsReturnedWhenNoMetadataIsLoaded )
			{
				// INIT
				shared_ptr<symbol_resolver> r(new symbol_resolver(get_requestor()));

				// ACT / ASSERT
				assert_is_empty(r->symbol_name_by_va(0x00F));
				assert_is_empty(r->symbol_name_by_va(0x100));
			}


			test( EmptyNameIsReturnedForFunctionsBeforeTheirBegining )
			{
				// INIT
				shared_ptr<symbol_resolver> r(new symbol_resolver(get_requestor()));
				mapped_module_identified basic = { };
				symbol_info symbols[] = { { "foo", 0x010, 3 }, { "bar", 0x101, 5 }, };
				module_info_metadata metadata = { mkvector(symbols), };

				r->add_mapping(basic);
				r->add_metadata(basic.persistent_id, metadata);

				// ACT / ASSERT
				assert_is_empty(r->symbol_name_by_va(0x00F));
				assert_is_empty(r->symbol_name_by_va(0x100));
			}


			test( EmptyNameIsReturnedForFunctionsPassTheEnd )
			{
				// INIT
				shared_ptr<symbol_resolver> r(new symbol_resolver(get_requestor()));
				mapped_module_identified basic = { };
				symbol_info symbols[] = { { "foo", 0x010, 3 }, { "bar", 0x101, 5 }, };
				module_info_metadata metadata = { mkvector(symbols), };

				r->add_mapping(basic);
				r->add_metadata(basic.persistent_id, metadata);

				// ACT / ASSERT
				assert_is_empty(r->symbol_name_by_va(0x013));
				assert_is_empty(r->symbol_name_by_va(0x107));
			}


			test( FunctionsLoadedThroughAsMetadataAreSearchableByAbsoluteAddress )
			{
				// INIT
				shared_ptr<symbol_resolver> r(new symbol_resolver(get_requestor()));
				mapped_module_identified basic = { };
				symbol_info symbols[] = { { "foo", 0x1010, 3 }, { "bar_2", 0x1101, 5 }, };
				module_info_metadata metadata = { mkvector(symbols), };

				// ACT
				r->add_mapping(basic);
				r->add_metadata(basic.persistent_id, metadata);

				// ASSERT
				assert_equal("foo", r->symbol_name_by_va(0x1010));
				assert_equal("foo", r->symbol_name_by_va(0x1012));
				assert_equal("bar_2", r->symbol_name_by_va(0x1101));
				assert_equal("bar_2", r->symbol_name_by_va(0x1105));
			}

			test( FunctionsLoadedThroughAsMetadataAreOffsetAccordingly )
			{
				// INIT
				shared_ptr<symbol_resolver> r(new symbol_resolver(get_requestor()));
				mapped_module_identified basic = create_mapping(1u, 0x1100000);
				symbol_info symbols[] = { { "foo", 0x010, 3 }, { "bar", 0x101, 5 }, };
				module_info_metadata metadata = { mkvector(symbols), };

				r->add_metadata(1, metadata);

				// ACT
				r->add_mapping(basic);

				// ASSERT
				assert_equal("foo", r->symbol_name_by_va(0x1100010));
				assert_equal("bar", r->symbol_name_by_va(0x1100101));

				// INIT
				basic.base = 0x1100501;

				// ACT
				r->add_mapping(basic);

				// ASSERT
				assert_equal("foo", r->symbol_name_by_va(0x1100010));
				assert_equal("bar", r->symbol_name_by_va(0x1100101));

				assert_equal("foo", r->symbol_name_by_va(0x1100511));
				assert_equal("bar", r->symbol_name_by_va(0x1100602));
			}


			test( ConstantReferenceFromResolverIsTheSame )
			{
				// INIT
				shared_ptr<symbol_resolver> r(new symbol_resolver(get_requestor()));
				mapped_module_identified basic = { };
				symbol_info symbols[] = { { "foo", 0x010, 3 }, { "bar", 0x101, 5 }, { "baz", 0x108, 5 }, };
				module_info_metadata metadata = { mkvector(symbols), };

				r->add_mapping(basic);
				r->add_metadata(basic.persistent_id, metadata);

				// ACT
				const string *name1_1 = &r->symbol_name_by_va(0x0010);
				const string *name2_1 = &r->symbol_name_by_va(0x0101);
				const string *name3_1 = &r->symbol_name_by_va(0x0108);

				const string *name2_2 = &r->symbol_name_by_va(0x101);
				const string *name1_2 = &r->symbol_name_by_va(0x010);
				const string *name3_2 = &r->symbol_name_by_va(0x108);

				// ASSERT
				assert_equal(name1_1, name1_2);
				assert_equal(name2_1, name2_2);
				assert_equal(name3_1, name3_2);
			}


			test( NoFileLineInformationIsReturnedForInvalidAddress )
			{
				// INIT
				shared_ptr<symbol_resolver> r(new symbol_resolver(get_requestor()));
				symbol_resolver::fileline_t dummy;

				// ACT / ASSERT
				assert_is_false(r->symbol_fileline_by_va(0, dummy));
			}


			test( FileLineInformationIsReturnedBySymolProvider )
			{
				// INIT
				shared_ptr<symbol_resolver> r(new symbol_resolver(get_requestor()));
				mapped_module_identified basic = { };
				symbol_info symbols[] = {
					{ "a", 0x010, 3, 11, 121 },
					{ "b", 0x101, 5, 1, 1 },
					{ "c", 0x158, 5, 7, 71 },
					{ "d", 0x188, 5, 7, 713 },
				};
				pair<unsigned, string> files[] = {
					make_pair(11, "zoo.cpp"), make_pair(7, "c:/umi.cpp"), make_pair(1, "d:\\dev\\kloo.cpp"),
				};
				module_info_metadata metadata = { mkvector(symbols), mkvector(files) };
				symbol_resolver::fileline_t results[4];

				r->add_mapping(basic);
				r->add_metadata(basic.persistent_id, metadata);

				// ACT / ASSERT
				assert_is_true(r->symbol_fileline_by_va(0x010, results[0]));
				assert_is_true(r->symbol_fileline_by_va(0x101, results[1]));
				assert_is_true(r->symbol_fileline_by_va(0x158, results[2]));
				assert_is_true(r->symbol_fileline_by_va(0x188, results[3]));

				// ASSERT
				symbol_resolver::fileline_t reference[] = {
					make_pair("zoo.cpp", 121),
					make_pair("d:\\dev\\kloo.cpp", 1),
					make_pair("c:/umi.cpp", 71),
					make_pair("c:/umi.cpp", 713),
				};

				assert_equal(reference, results);
			}


			test( FileLineInformationIsRebasedForNewMetadata )
			{
				// INIT
				shared_ptr<symbol_resolver> r(new symbol_resolver(get_requestor()));
				mapped_module_identified basic = { };
				symbol_info symbols1[] = {
					{ "a", 0x010, 3, 11, 121 },
					{ "b", 0x101, 5, 1, 1 },
					{ "c", 0x158, 5, 7, 71 },
					{ "d", 0x188, 5, 7, 713 },
				};
				symbol_info symbols2[] = {
					{ "a", 0x010, 3, 1, 121 },
					{ "b", 0x101, 5, 1, 1 },
					{ "c", 0x158, 5, 7, 71 },
					{ "d", 0x180, 5, 50, 71 },
				};
				pair<unsigned, string> files1[] = {
					make_pair(11, "zoo.cpp"), make_pair(1, "c:/umi.cpp"), make_pair(7, "d:\\dev\\kloo.cpp"),
				};
				pair<unsigned, string> files2[] = {
					make_pair(1, "ZOO.cpp"), make_pair(7, "/projects/umi.cpp"), make_pair(50, "d:\\dev\\abc.cpp"),
				};
				module_info_metadata metadata[] = { 
					{ mkvector(symbols1), mkvector(files1) },
					{ mkvector(symbols2), mkvector(files2) },
				};
				symbol_resolver::fileline_t results[8];

				basic.persistent_id = 1;
				r->add_mapping(basic);
				r->add_metadata(basic.persistent_id, metadata[0]);

				basic.persistent_id = 2;
				basic.base = 0x8000;

				// ACT / ASSERT
				r->add_mapping(basic);
				r->add_metadata(basic.persistent_id, metadata[1]);
				r->symbol_fileline_by_va(0x010, results[0]);
				r->symbol_fileline_by_va(0x101, results[1]);
				r->symbol_fileline_by_va(0x158, results[2]);
				r->symbol_fileline_by_va(0x188, results[3]);
				r->symbol_fileline_by_va(0x8010, results[4]);
				r->symbol_fileline_by_va(0x8101, results[5]);
				r->symbol_fileline_by_va(0x8158, results[6]);
				r->symbol_fileline_by_va(0x8180, results[7]);

				// ASSERT
				symbol_resolver::fileline_t reference[] = {
					make_pair("zoo.cpp", 121),
					make_pair("c:/umi.cpp", 1),
					make_pair("d:\\dev\\kloo.cpp", 71),
					make_pair("d:\\dev\\kloo.cpp", 713),
					make_pair("ZOO.cpp", 121),
					make_pair("ZOO.cpp", 1),
					make_pair("/projects/umi.cpp", 71),
					make_pair("d:\\dev\\abc.cpp", 71),
				};

				assert_equal(reference, results);
			}


			test( SymbolResolverIsSerializable )
			{
				// INIT
				vector_adapter buffer;
				strmd::serializer<vector_adapter, packer> ser(buffer);
				strmd::deserializer<vector_adapter, packer> dser(buffer);
				shared_ptr<symbol_resolver> r(new symbol_resolver(get_requestor()));
				mapped_module_identified basic = { };
				symbol_info symbols[] = {
					{ "a", 0x010, 3, 11, 121 },
					{ "b", 0x101, 5, 1, 1 },
					{ "cccc", 0x158, 5, 7, 71 },
					{ "ddd", 0x188, 5, 7, 713 },
				};
				pair<unsigned, string> files[] = {
					make_pair(11, "zoo.cpp"), make_pair(1, "c:/umi.cpp"), make_pair(7, "d:\\dev\\kloo.cpp"),
				};
				module_info_metadata metadata = { mkvector(symbols), mkvector(files) };
				symbol_resolver::fileline_t results[4];

				r->add_mapping(basic);
				r->add_metadata(basic.persistent_id, metadata);

				// ACT
				shared_ptr<symbol_resolver> r2(new symbol_resolver(get_requestor()));

				ser(*r);
				dser(*r2);

				// ASSERT
				assert_equal("a", r2->symbol_name_by_va(0x010));
				assert_equal("b", r2->symbol_name_by_va(0x101));
				assert_equal("cccc", r2->symbol_name_by_va(0x158));
				assert_equal("ddd", r2->symbol_name_by_va(0x188));

				assert_is_true(r2->symbol_fileline_by_va(0x010, results[0]));
				assert_is_true(r2->symbol_fileline_by_va(0x101, results[1]));
				assert_is_true(r2->symbol_fileline_by_va(0x158, results[2]));
				assert_is_true(r2->symbol_fileline_by_va(0x188, results[3]));

				symbol_resolver::fileline_t reference[] = {
					make_pair("zoo.cpp", 121),
					make_pair("c:/umi.cpp", 1),
					make_pair("d:\\dev\\kloo.cpp", 71),
					make_pair("d:\\dev\\kloo.cpp", 713),
				};

				assert_equal(reference, results);
			}


			test( FilelineIsNotAvailableForMissingFunctions )
			{
				// INIT
				symbol_resolver r(get_requestor());
				symbol_resolver::fileline_t result;

				// ACT / ASSERT
				assert_is_false(r.symbol_fileline_by_va(123, result));
			}


			test( FilelineIsNotAvailableForFunctionsWithMissingFiles )
			{
				// INIT
				symbol_resolver r(get_requestor());
				symbol_info symbols[] = {
					{ "a", 0x010, 3, 11, 121 },
					{ "b", 0x101, 5, 1, 1 },
					{ "cccc", 0x158, 5, 0, 71 },
					{ "ddd", 0x188, 5, 0, 713 },
				};
				mapped_module_identified basic = { };
				module_info_metadata metadata = { mkvector(symbols), };
				symbol_resolver::fileline_t result;

				r.add_mapping(basic);
				r.add_metadata(basic.persistent_id, metadata);

				// ACT / ASSERT
				assert_is_false(r.symbol_fileline_by_va(0x010, result));
				assert_is_false(r.symbol_fileline_by_va(0x101, result));
				assert_is_false(r.symbol_fileline_by_va(0x158, result));
				assert_is_false(r.symbol_fileline_by_va(0x188, result));
			}


			test( SymbolResolverRequestsMetadataOnFindSymbolHits )
			{
				// INIT
				vector_adapter buffer;
				shared_ptr<symbol_resolver> r(new symbol_resolver(get_requestor()));

				r->add_mapping(create_mapping(11, 0x120050));
				r->add_mapping(create_mapping(12, 0x120100));
				r->add_mapping(create_mapping(11711, 0x200000));
				r->add_mapping(create_mapping(100, 0x310000));

				// ACT
				r->symbol_name_by_va(0x120060);

				// ASSERT
				unsigned reference1[] = { 11, };

				assert_equal(reference1, _requested);

				// ACT
				r->symbol_name_by_va(0x311000);
				r->symbol_name_by_va(0x210000);

				// ASSERT
				unsigned reference2[] = { 11, 100, 11711, };

				assert_equal(reference2, _requested);
			}


			test( MetadataIsNotRequestedIfAlreadyPresent )
			{
				// INIT
				vector_adapter buffer;
				shared_ptr<symbol_resolver> r(new symbol_resolver(get_requestor()));
				module_info_metadata metadata;

				r->add_mapping(create_mapping(100, 0x120050));
				r->add_mapping(create_mapping(2, 0x120100));

				r->add_metadata(100, metadata);

				// ACT
				r->symbol_name_by_va(0x1200A0);

				// ASSERT
				assert_is_empty(_requested);
			}


			test( MetadataIsOnlyRequestedOncePerModule )
			{
				// INIT
				vector_adapter buffer;
				shared_ptr<symbol_resolver> r(new symbol_resolver(get_requestor()));
				module_info_metadata metadata;

				r->add_mapping(create_mapping(100, 0x120050));
				r->add_mapping(create_mapping(2, 0x120100));

				// ACT
				r->symbol_name_by_va(0x120100);
				r->symbol_name_by_va(0x120050);
				r->symbol_name_by_va(0x1200A0);
				r->symbol_name_by_va(0x1200A0);
				r->symbol_name_by_va(0x120101);

				// ASSERT
				unsigned reference[] = { 2, 100, };

				assert_equal(reference, _requested);
			}


		end_test_suite
	}
}
