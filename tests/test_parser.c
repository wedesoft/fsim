#include "fsim/parser.h"
#include "fsim/list.h"
#include "fsim/hash.h"
#include "test_helper.h"


extern object_t *parse_string_core(const char *text);
extern object_t *parse_result;
extern hash_t *parse_materials;
extern list_t *parse_vertex;
extern list_t *parse_uv;
extern list_t *parse_normal;


static MunitResult test_empty(const MunitParameter params[], void *data)
{
  munit_assert_ptr(parse_string(""), ==, NULL);
  return MUNIT_OK;
}

static MunitResult test_object(const MunitParameter params[], void *data)
{
  munit_assert_ptr(parse_string("o test"), !=, NULL);
  return MUNIT_OK;
}

static MunitResult test_object_name(const MunitParameter params[], void *data)
{
  object_t *object = parse_string("o test");
  munit_assert_string_equal(object->name, "test");
  return MUNIT_OK;
}

static MunitResult test_ignore_whitespace(const MunitParameter params[], void *data)
{
  munit_assert_string_equal(parse_string("o two words")->name, "two words");
  return MUNIT_OK;
}

static MunitResult test_whitespace_in_name(const MunitParameter params[], void *data)
{
  munit_assert_string_equal(parse_string("o  test")->name, "test");
  return MUNIT_OK;
}

static MunitResult test_handle_ctrl_lf(const MunitParameter params[], void *data)
{
  munit_assert_string_equal(parse_string("o test\r\n")->name, "test");
  return MUNIT_OK;
}

static MunitResult test_error(const MunitParameter params[], void *data)
{
  munit_assert_ptr(parse_string("o test\n?"), ==, NULL);
  return MUNIT_OK;
}

static MunitResult test_read_file(const MunitParameter params[], void *data)
{
  munit_assert_string_equal(parse_file("name.obj")->name, "inafile");
  return MUNIT_OK;
}

static MunitResult test_no_such_file(const MunitParameter params[], void *data)
{
  munit_assert_ptr(parse_file("nosuchfile.obj"), ==, NULL);
  return MUNIT_OK;
}

static MunitResult test_ignore_comments(const MunitParameter params[], void *data)
{
  munit_assert_ptr(parse_string("# Test comment\no test\n "), !=, NULL);
  return MUNIT_OK;
}

static MunitResult test_ignore_trailing_comment(const MunitParameter params[], void *data)
{
  munit_assert_ptr(parse_string("o test\nv 1 2 3\n# Test comment"), !=, NULL);
  return MUNIT_OK;
}

static MunitResult test_no_newline_in_name(const MunitParameter params[], void *data)
{
  munit_assert_string_equal(parse_string("o test\n")->name, "test");
  return MUNIT_OK;
}

static MunitResult test_no_vertices(const MunitParameter params[], void *data)
{
  parse_string_core("");
  munit_assert_int(parse_vertex->size, ==, 0);
  return MUNIT_OK;
}

static MunitResult test_read_vertex(const MunitParameter params[], void *data)
{
  parse_string_core("o test\nv 10 2.5 -3.25");
  munit_assert_int(parse_vertex->size, ==, 3);
  munit_assert_float(get_glfloat(parse_vertex)[0], ==, 10.0f);
  munit_assert_float(get_glfloat(parse_vertex)[1], ==,  2.5f);
  munit_assert_float(get_glfloat(parse_vertex)[2], ==, -3.25f);
  return MUNIT_OK;
}

static MunitResult test_two_vertices(const MunitParameter params[], void *data)
{
  parse_string_core("o test\nv 10 2.5 -3.25\nv .5 +1 -.25");
  munit_assert_int(parse_vertex->size, ==, 6);
  munit_assert_float(get_glfloat(parse_vertex)[3], ==,  0.5f);
  munit_assert_float(get_glfloat(parse_vertex)[4], ==,  1.0f);
  munit_assert_float(get_glfloat(parse_vertex)[5], ==, -0.25f);
  return MUNIT_OK;
}

static MunitResult test_exponent(const MunitParameter params[], void *data)
{
  parse_string_core("o test\nv 1e+1 2.5e-1 -3.0e2");
  munit_assert_int(parse_vertex->size, ==, 3);
  munit_assert_float(get_glfloat(parse_vertex)[0], ==,  10.0f);
  munit_assert_float(get_glfloat(parse_vertex)[1], ==,   0.25f);
  munit_assert_float(get_glfloat(parse_vertex)[2], ==, -300.0f);
  return MUNIT_OK;
}

static MunitResult test_cleanup_vertices(const MunitParameter params[], void *data)
{
  parse_string("o test\nv 10 2.5 3.25");
  munit_assert_ptr(parse_vertex, ==, NULL);
  return MUNIT_OK;
}

static MunitResult test_no_groups(const MunitParameter params[], void *data)
{
  munit_assert_int(parse_string("o test\n")->group->size, ==, 0);
  return MUNIT_OK;
}

static MunitResult test_start_group(const MunitParameter params[], void *data)
{
  munit_assert_int(parse_string("o test\ng group")->group->size, ==, 1);
  return MUNIT_OK;
}

static MunitResult test_group_name(const MunitParameter params[], void *data)
{
  group_t *group = get_pointer(parse_string("o test\ng test group")->group)[0];
  munit_assert_string_equal(group->name, "test group");
  return MUNIT_OK;
}

static MunitResult test_two_groups(const MunitParameter params[], void *data)
{
  parse_string_core("o test\ng group\ng group");
  munit_assert_int(parse_result->group->size, ==, 2);
  return MUNIT_OK;
}

static MunitResult test_default_object(const MunitParameter params[], void *data)
{
  munit_assert_string_equal(parse_string("g group")->name, "");
  return MUNIT_OK;
}

static MunitResult test_facet(const MunitParameter params[], void *data)
{
  parse_string_core("o test\nv 2 3 5\nv 3 5 7\nv 7 5 3\ng group\nf 1 2 3");
  group_t *group = get_pointer(parse_result->group)[0];
  munit_assert_int(group->vertex_index->size, ==, 3);
  return MUNIT_OK;
}

static MunitResult test_vertex_stride(const MunitParameter params[], void *data)
{
  parse_string_core("o test\nv 2 3 5\nv 3 5 7\nv 7 5 3\ng group\nf 1 2 3");
  group_t *group = get_pointer(parse_result->group)[0];
  munit_assert_int(group->stride, ==, 3);
  return MUNIT_OK;
}

static MunitResult test_indices(const MunitParameter params[], void *data)
{
  parse_string_core("o test\nv 2 3 5\nv 3 5 7\nv 7 5 3\ng group\nf 1 2 3");
  group_t *group = get_pointer(parse_result->group)[0];
  munit_assert_int(get_gluint(group->vertex_index)[0], ==, 0);
  munit_assert_int(get_gluint(group->vertex_index)[1], ==, 1);
  munit_assert_int(get_gluint(group->vertex_index)[2], ==, 2);
  return MUNIT_OK;
}

static MunitResult test_coord_count(const MunitParameter params[], void *data)
{
  parse_string_core("o test\nv 2 3 5\nv 3 5 7\nv 7 5 3\ng group\nf 1 2 3");
  group_t *group = get_pointer(parse_result->group)[0];
  munit_assert_int(group->array->size, ==, 9);
  return MUNIT_OK;
}

static MunitResult test_copy_coords(const MunitParameter params[], void *data)
{
  parse_string_core("o test\nv 2 3 5\nv 3 5 7\nv 7 5 3\ng group\nf 1 2 3");
  group_t *group = get_pointer(parse_result->group)[0];
  munit_assert_float(get_glfloat(group->array)[0], ==, 2.0f);
  munit_assert_float(get_glfloat(group->array)[1], ==, 3.0f);
  munit_assert_float(get_glfloat(group->array)[2], ==, 5.0f);
  munit_assert_float(get_glfloat(group->array)[3], ==, 3.0f);
  munit_assert_float(get_glfloat(group->array)[4], ==, 5.0f);
  munit_assert_float(get_glfloat(group->array)[5], ==, 7.0f);
  munit_assert_float(get_glfloat(group->array)[6], ==, 7.0f);
  munit_assert_float(get_glfloat(group->array)[7], ==, 5.0f);
  munit_assert_float(get_glfloat(group->array)[8], ==, 3.0f);
  return MUNIT_OK;
}

static MunitResult test_shuffle_coords(const MunitParameter params[], void *data)
{
  parse_string_core("o test\nv 2 3 5\nv 3 5 7\nv 7 5 3\ng group\nf 1 3 2");
  group_t *group = get_pointer(parse_result->group)[0];
  munit_assert_float(get_glfloat(group->array)[0], ==, 2.0f);
  munit_assert_float(get_glfloat(group->array)[1], ==, 3.0f);
  munit_assert_float(get_glfloat(group->array)[2], ==, 5.0f);
  munit_assert_float(get_glfloat(group->array)[3], ==, 7.0f);
  munit_assert_float(get_glfloat(group->array)[4], ==, 5.0f);
  munit_assert_float(get_glfloat(group->array)[5], ==, 3.0f);
  munit_assert_float(get_glfloat(group->array)[6], ==, 3.0f);
  munit_assert_float(get_glfloat(group->array)[7], ==, 5.0f);
  munit_assert_float(get_glfloat(group->array)[8], ==, 7.0f);
  return MUNIT_OK;
}

static MunitResult test_relative_index(const MunitParameter params[], void *data)
{
  parse_string_core("o test\nv 2 3 5\nv 3 5 7\nv 7 5 3\ng group\nf -3 -2 -1");
  group_t *group = get_pointer(parse_result->group)[0];
  munit_assert_float(get_glfloat(group->array)[0], ==, 2.0f);
  munit_assert_float(get_glfloat(group->array)[1], ==, 3.0f);
  munit_assert_float(get_glfloat(group->array)[2], ==, 5.0f);
  munit_assert_float(get_glfloat(group->array)[3], ==, 3.0f);
  munit_assert_float(get_glfloat(group->array)[4], ==, 5.0f);
  munit_assert_float(get_glfloat(group->array)[5], ==, 7.0f);
  munit_assert_float(get_glfloat(group->array)[6], ==, 7.0f);
  munit_assert_float(get_glfloat(group->array)[7], ==, 5.0f);
  munit_assert_float(get_glfloat(group->array)[8], ==, 3.0f);
  return MUNIT_OK;
}

static MunitResult test_generate_indices(const MunitParameter params[], void *data)
{
  parse_string_core("o test\nv 2 3 5\nv 3 5 7\nv 7 5 3\ng group\nf 1 3 2");
  group_t *group = get_pointer(parse_result->group)[0];
  munit_assert_int(get_gluint(group->vertex_index)[0], ==, 0);
  munit_assert_int(get_gluint(group->vertex_index)[1], ==, 1);
  munit_assert_int(get_gluint(group->vertex_index)[2], ==, 2);
  return MUNIT_OK;
}

static MunitResult test_reset_vertex_cache(const MunitParameter params[], void *data)
{
  parse_string_core("o test\nv 2 3 5\nv 3 5 7\nv 7 5 3\ng group\nf 1 3 2\ng group\nf 3 2 1");
  group_t *group = get_pointer(parse_result->group)[1];
  munit_assert_int(group->array->size, ==, 9);
  return MUNIT_OK;
}

static MunitResult test_two_facets(const MunitParameter params[], void *data)
{
  parse_string_core("o test\nv 2 3 5\nv 3 5 7\nv 7 5 3\nv 9 7 5\ng group\nf 1 2 3\nf 1 4 3");
  group_t *group = get_pointer(parse_result->group)[0];
  munit_assert_int(group->vertex_index->size, ==, 6);
  return MUNIT_OK;
}

static MunitResult test_reuse_vertices(const MunitParameter params[], void *data)
{
  parse_string_core("o test\nv 2 3 5\nv 3 5 7\nv 7 5 3\nv 9 7 5\ng group\nf 1 2 3\nf 4 3 1");
  group_t *group = get_pointer(parse_result->group)[0];
  munit_assert_int(get_gluint(group->vertex_index)[0], ==, 0);
  munit_assert_int(get_gluint(group->vertex_index)[1], ==, 1);
  munit_assert_int(get_gluint(group->vertex_index)[2], ==, 2);
  munit_assert_int(get_gluint(group->vertex_index)[3], ==, 3);
  munit_assert_int(get_gluint(group->vertex_index)[4], ==, 2);
  munit_assert_int(get_gluint(group->vertex_index)[5], ==, 0);
  return MUNIT_OK;
}

static MunitResult test_square_facet(const MunitParameter params[], void *data)
{
  parse_string_core("o test\nv 2 3 5\nv 3 5 7\nv 7 5 3\nv 9 7 5\ng group\nf 1 2 3 4");
  group_t *group = get_pointer(parse_result->group)[0];
  munit_assert_int(get_gluint(group->vertex_index)[0], ==, 0);
  munit_assert_int(get_gluint(group->vertex_index)[1], ==, 1);
  munit_assert_int(get_gluint(group->vertex_index)[2], ==, 2);
  munit_assert_int(get_gluint(group->vertex_index)[3], ==, 0);
  munit_assert_int(get_gluint(group->vertex_index)[4], ==, 2);
  munit_assert_int(get_gluint(group->vertex_index)[5], ==, 3);
  return MUNIT_OK;
}

static MunitResult test_no_group(const MunitParameter params[], void *data)
{
  object_t *object = parse_string("o test");
  munit_assert_int(object->group->size, ==, 0);
  return MUNIT_OK;
}

static MunitResult test_add_group(const MunitParameter params[], void *data)
{
  object_t *object = parse_string("o test\nv 2 3 5\nv 3 5 7\nv 7 5 3\ng group\nf 1 2 3");
  munit_assert_int(object->group->size, ==, 1);
  return MUNIT_OK;
}

static MunitResult test_no_texcoord(const MunitParameter params[], void *data)
{
  parse_string_core("");
  munit_assert_int(parse_uv->size, ==, 0);
  return MUNIT_OK;
}

static MunitResult test_cleanup_texcoords(const MunitParameter params[], void *data)
{
  parse_string("o test");
  munit_assert_ptr(parse_uv, ==, NULL);
  return MUNIT_OK;
}

static MunitResult test_read_texcoord(const MunitParameter params[], void *data)
{
  parse_string_core("o test\nvt 0.5 0.25");
  munit_assert_int(parse_uv->size, ==, 2);
  munit_assert_float(get_glfloat(parse_uv)[0], ==, 0.50f);
  munit_assert_float(get_glfloat(parse_uv)[1], ==, 0.25f);
  return MUNIT_OK;
}

static MunitResult test_two_texcoords(const MunitParameter params[], void *data)
{
  parse_string_core("o test\nvt 0.5 0.25\nvt 0.75 1.0");
  munit_assert_int(parse_uv->size, ==, 4);
  munit_assert_float(get_glfloat(parse_uv)[2], ==, 0.75f);
  munit_assert_float(get_glfloat(parse_uv)[3], ==, 1.00f);
  return MUNIT_OK;
}

static MunitResult test_uv_facet(const MunitParameter params[], void *data)
{
  parse_string_core("o test\nv 2 2 -1\nv 2 3 -1\nv 3 2 -1\nvt 0 0\nvt 0 1\nvt 1 0\ng group\nf 1/1 2/2 3/3");
  group_t *group = get_pointer(parse_result->group)[0];
  munit_assert_int(group->vertex_index->size, ==, 3);
  return MUNIT_OK;
}

static MunitResult test_vertex_and_uv_stride(const MunitParameter params[], void *data)
{
  parse_string_core("o test\nv 2 2 -1\nv 2 3 -1\nv 3 2 -1\nvt 0 0\nvt 0 1\nvt 1 0\ng group\nf 1/1 2/2 3/3");
  group_t *group = get_pointer(parse_result->group)[0];
  munit_assert_int(group->stride, ==, 5);
  return MUNIT_OK;
}

static MunitResult test_uv_indices(const MunitParameter params[], void *data)
{
  parse_string_core("o test\nv 2 2 -1\nv 2 3 -1\nv 3 2 -1\nvt 0 0\nvt 0 1\nvt 1 0\ng group\nf 1/1 2/2 3/3");
  group_t *group = get_pointer(parse_result->group)[0];
  munit_assert_int(get_gluint(group->vertex_index)[0], ==, 0);
  munit_assert_int(get_gluint(group->vertex_index)[1], ==, 1);
  munit_assert_int(get_gluint(group->vertex_index)[2], ==, 2);
  return MUNIT_OK;
}

static MunitResult test_uv_added(const MunitParameter params[], void *data)
{
  parse_string_core("o test\nv 2 2 -1\nv 2 3 -1\nv 3 2 -1\nvt 0 0\nvt 0 1\nvt 1 0\ng group\nf 1/1 2/2 3/3");
  group_t *group = get_pointer(parse_result->group)[0];
  munit_assert_int(group->array->size, ==, 15);
  return MUNIT_OK;
}

static MunitResult test_copy_uv(const MunitParameter params[], void *data)
{
  parse_string_core("o test\nv 2 2 -1\nv 2 3 -1\nv 3 2 -1\nvt 0 0\nvt 0 1\nvt 1 0\ng group\nf 1/1 2/2 3/3");
  group_t *group = get_pointer(parse_result->group)[0];
  munit_assert_float(get_glfloat(group->array)[0], ==, 2.0f);
  munit_assert_float(get_glfloat(group->array)[1], ==, 2.0f);
  munit_assert_float(get_glfloat(group->array)[2], ==,-1.0f);
  munit_assert_float(get_glfloat(group->array)[3], ==, 0.0f);
  munit_assert_float(get_glfloat(group->array)[4], ==, 0.0f);
  munit_assert_float(get_glfloat(group->array)[5], ==, 2.0f);
  munit_assert_float(get_glfloat(group->array)[6], ==, 3.0f);
  munit_assert_float(get_glfloat(group->array)[7], ==,-1.0f);
  munit_assert_float(get_glfloat(group->array)[8], ==, 0.0f);
  munit_assert_float(get_glfloat(group->array)[9], ==, 1.0f);
  return MUNIT_OK;
}

static MunitResult test_relative_uv_indices(const MunitParameter params[], void *data)
{
  parse_string_core("o test\nv 2 2 -1\nv 2 3 -1\nv 3 2 -1\nvt 0 0\nvt 0 1\nvt 1 0\ng group\nf -3/-3 -2/-2 -1/-1");
  group_t *group = get_pointer(parse_result->group)[0];
  munit_assert_float(get_glfloat(group->array)[3], ==, 0.0f);
  munit_assert_float(get_glfloat(group->array)[4], ==, 0.0f);
  munit_assert_float(get_glfloat(group->array)[8], ==, 0.0f);
  munit_assert_float(get_glfloat(group->array)[9], ==, 1.0f);
  return MUNIT_OK;
}

static MunitResult test_different_uv_index(const MunitParameter params[], void *data)
{
  parse_string_core("o test\nv 2 2 -1\nv 2 3 -1\nv 3 2 -1\nvt 0 0\nvt 0 1\nvt 1 0\ng group\nf 1/2 2/3 3/1");
  group_t *group = get_pointer(parse_result->group)[0];
  munit_assert_float(get_glfloat(group->array)[0], ==, 2.0f);
  munit_assert_float(get_glfloat(group->array)[1], ==, 2.0f);
  munit_assert_float(get_glfloat(group->array)[2], ==,-1.0f);
  munit_assert_float(get_glfloat(group->array)[3], ==, 0.0f);
  munit_assert_float(get_glfloat(group->array)[4], ==, 1.0f);
  return MUNIT_OK;
}

static MunitResult test_reuse_uv(const MunitParameter params[], void *data)
{
  parse_string_core("o test\nv 2 2 -1\nv 2 3 -1\nv 3 2 -1\nvt 0 0\nvt 0 1\nvt 1 0\ng group\nf 1/1 1/1 2/2");
  group_t *group = get_pointer(parse_result->group)[0];
  munit_assert_int(group->array->size, ==, 10);
  return MUNIT_OK;
}

static MunitResult test_uv_in_hash_key(const MunitParameter params[], void *data)
{
  parse_string_core("o test\nv 2 2 -1\nv 2 3 -1\nv 3 2 -1\nvt 0 0\nvt 0 1\nvt 1 0\ng group\nf 1/1 1/2 1/3");
  group_t *group = get_pointer(parse_result->group)[0];
  munit_assert_int(group->array->size, ==, 15);
  return MUNIT_OK;
}

static MunitResult test_vertex_and_uv_key(const MunitParameter params[], void *data)
{
  parse_string_core("o test\nv 2 2 -1\nv 2 3 -1\nv 3 2 -1\nvt 0 0\nvt 0 1\nvt 1 0\ng group\nf 1/1 2/1 3/1");
  group_t *group = get_pointer(parse_result->group)[0];
  munit_assert_int(group->array->size, ==, 15);
  return MUNIT_OK;
}

static MunitResult test_no_normal(const MunitParameter params[], void *data)
{
  parse_string_core("");
  munit_assert_int(parse_normal->size, ==, 0);
  return MUNIT_OK;
}

static MunitResult test_cleanup_normals(const MunitParameter params[], void *data)
{
  parse_string("");
  munit_assert_ptr(parse_normal, ==, NULL);
  return MUNIT_OK;
}

static MunitResult test_read_normal(const MunitParameter params[], void *data)
{
  parse_string_core("o test\nvn 0.36 0.48 0.8");
  munit_assert_int(parse_normal->size, ==, 3);
  munit_assert_float(get_glfloat(parse_normal)[0], ==, 0.36f);
  munit_assert_float(get_glfloat(parse_normal)[1], ==, 0.48f);
  munit_assert_float(get_glfloat(parse_normal)[2], ==, 0.80f);
  return MUNIT_OK;
}

static MunitResult test_normal_facet(const MunitParameter params[], void *data)
{
  parse_string_core("o test\nv 2 2 -1\nv 2 3 -1\nv 3 2 -1\nvn 1 0 0\nvn 0 1 0\nvn 0 0 1\ng group\nf 1//1 2//2 3//3");
  group_t *group = get_pointer(parse_result->group)[0];
  munit_assert_int(group->vertex_index->size, ==, 3);
  return MUNIT_OK;
}

static MunitResult test_normal_indices(const MunitParameter params[], void *data)
{
  parse_string_core("o test\nv 2 2 -1\nv 2 3 -1\nv 3 2 -1\nvn 1 0 0\nvn 0 1 0\nvn 0 0 1\ng group\nf 1//1 2//2 3//3");
  group_t *group = get_pointer(parse_result->group)[0];
  munit_assert_int(get_gluint(group->vertex_index)[0], ==, 0);
  munit_assert_int(get_gluint(group->vertex_index)[1], ==, 1);
  munit_assert_int(get_gluint(group->vertex_index)[2], ==, 2);
  return MUNIT_OK;
}

static MunitResult test_normals_added(const MunitParameter params[], void *data)
{
  parse_string_core("o test\nv 2 2 -1\nv 2 3 -1\nv 3 2 -1\nvn 1 0 0\nvn 0 1 0\nvn 0 0 1\ng group\nf 1//1 2//2 3//3");
  group_t *group = get_pointer(parse_result->group)[0];
  munit_assert_int(group->array->size, ==, 18);
  return MUNIT_OK;
}

static MunitResult test_copy_normal(const MunitParameter params[], void *data)
{
  parse_string_core("o test\nv 2 2 -1\nv 2 3 -1\nv 3 2 -1\nvn 1 0 0\nvn 0 1 0\nvn 0 0 1\ng group\nf 1//1 2//2 3//3");
  group_t *group = get_pointer(parse_result->group)[0];
  munit_assert_float(get_glfloat(group->array)[ 0], ==, 2.0f);
  munit_assert_float(get_glfloat(group->array)[ 1], ==, 2.0f);
  munit_assert_float(get_glfloat(group->array)[ 2], ==,-1.0f);
  munit_assert_float(get_glfloat(group->array)[ 3], ==, 1.0f);
  munit_assert_float(get_glfloat(group->array)[ 4], ==, 0.0f);
  munit_assert_float(get_glfloat(group->array)[ 5], ==, 0.0f);
  munit_assert_float(get_glfloat(group->array)[ 6], ==, 2.0f);
  munit_assert_float(get_glfloat(group->array)[ 7], ==, 3.0f);
  munit_assert_float(get_glfloat(group->array)[ 8], ==,-1.0f);
  munit_assert_float(get_glfloat(group->array)[ 9], ==, 0.0f);
  munit_assert_float(get_glfloat(group->array)[10], ==, 1.0f);
  munit_assert_float(get_glfloat(group->array)[11], ==, 0.0f);
  return MUNIT_OK;
}

static MunitResult test_relative_normal_index(const MunitParameter params[], void *data)
{
  parse_string_core("o test\nv 2 2 -1\nv 2 3 -1\nv 3 2 -1\nvn 1 0 0\nvn 0 1 0\nvn 0 0 1\ng group\nf -3//-3 -2//-2 -1//-1");
  group_t *group = get_pointer(parse_result->group)[0];
  munit_assert_float(get_glfloat(group->array)[ 3], ==, 1.0f);
  munit_assert_float(get_glfloat(group->array)[ 4], ==, 0.0f);
  munit_assert_float(get_glfloat(group->array)[ 5], ==, 0.0f);
  munit_assert_float(get_glfloat(group->array)[ 9], ==, 0.0f);
  munit_assert_float(get_glfloat(group->array)[10], ==, 1.0f);
  munit_assert_float(get_glfloat(group->array)[11], ==, 0.0f);
  return MUNIT_OK;
}

static MunitResult test_different_normal_index(const MunitParameter params[], void *data)
{
  parse_string_core("o test\nv 2 2 -1\nv 2 3 -1\nv 3 2 -1\nvn 1 0 0\nvn 0 1 0\nvn 0 0 1\ng group\nf 1//2 2//3 3//1");
  group_t *group = get_pointer(parse_result->group)[0];
  munit_assert_float(get_glfloat(group->array)[0], ==, 2.0f);
  munit_assert_float(get_glfloat(group->array)[1], ==, 2.0f);
  munit_assert_float(get_glfloat(group->array)[2], ==,-1.0f);
  munit_assert_float(get_glfloat(group->array)[3], ==, 0.0f);
  munit_assert_float(get_glfloat(group->array)[4], ==, 1.0f);
  munit_assert_float(get_glfloat(group->array)[5], ==, 0.0f);
  return MUNIT_OK;
}

static MunitResult test_reuse_normal(const MunitParameter params[], void *data)
{
  parse_string_core("o test\nv 2 2 -1\nv 2 3 -1\nv 3 2 -1\nvn 1 0 0\nvn 0 1 0\nvn 0 0 1\ng group\nf 1//1 1//1 2//2");
  group_t *group = get_pointer(parse_result->group)[0];
  munit_assert_int(group->array->size, ==, 12);
  return MUNIT_OK;
}

static MunitResult test_normal_in_hash_key(const MunitParameter params[], void *data)
{
  parse_string_core("o test\nv 2 2 -1\nv 2 3 -1\nv 3 2 -1\nvn 1 0 0\nvn 0 1 0\nvn 0 0 1\ng group\nf 1//1 1//2 1//3");
  group_t *group = get_pointer(parse_result->group)[0];
  munit_assert_int(group->array->size, ==, 18);
  return MUNIT_OK;
}

static MunitResult test_vertex_and_normal_key(const MunitParameter params[], void *data)
{
  parse_string_core("o test\nv 2 2 -1\nv 2 3 -1\nv 3 2 -1\nvn 1 0 0\nvn 0 1 0\nvn 0 0 1\ng group\nf 1//1 2//1 3//1");
  group_t *group = get_pointer(parse_result->group)[0];
  munit_assert_int(group->array->size, ==, 18);
  return MUNIT_OK;
}

static MunitResult test_full_facet(const MunitParameter params[], void *data)
{
  parse_string_core(
      "o test\nv 2 2 0\nv 2 3 0\nv 3 2 0\nvn 1 0 0\nvn 0 1 0\nvn 0 0 1\nvt 0 0\nvt 0 1\nvt 1 0\n"
      "g group\nf 1/1/1 2/2/2 3/3/3");
  group_t *group = get_pointer(parse_result->group)[0];
  munit_assert_int(group->vertex_index->size, ==, 3);
  return MUNIT_OK;
}

static MunitResult test_full_indices(const MunitParameter params[], void *data)
{
  parse_string_core(
      "o test\nv 2 2 0\nv 2 3 0\nv 3 2 0\nvn 1 0 0\nvn 0 1 0\nvn 0 0 1\nvt 0 0\nvt 0 1\nvt 1 0\n"
      "g group\nf 1/1/1 2/2/2 3/3/3");
  group_t *group = get_pointer(parse_result->group)[0];
  munit_assert_int(get_gluint(group->vertex_index)[0], ==, 0);
  munit_assert_int(get_gluint(group->vertex_index)[1], ==, 1);
  munit_assert_int(get_gluint(group->vertex_index)[2], ==, 2);
  return MUNIT_OK;
}

static MunitResult test_all_added(const MunitParameter params[], void *data)
{
  parse_string_core(
      "o test\nv 2 2 0\nv 2 3 0\nv 3 2 0\nvn 1 0 0\nvn 0 1 0\nvn 0 0 1\nvt 0 0\nvt 0 1\nvt 1 0\n"
      "g group\nf 1/1/1 2/2/2 3/3/3");
  group_t *group = get_pointer(parse_result->group)[0];
  munit_assert_int(group->array->size, ==, 24);
  return MUNIT_OK;
}

static MunitResult test_different_indices(const MunitParameter params[], void *data)
{
  parse_string_core(
      "o test\nv 2 2 0\nv 2 3 0\nv 3 2 0\nvn 1 0 0\nvn 0 1 0\nvn 0 0 1\nvt 0 0\nvt 0 1\nvt 1 0\n"
      "g group\nf 1/2/3 2/3/1 3/1/2");
  group_t *group = get_pointer(parse_result->group)[0];
  munit_assert_float(get_glfloat(group->array)[0], ==, 2.0f);
  munit_assert_float(get_glfloat(group->array)[1], ==, 2.0f);
  munit_assert_float(get_glfloat(group->array)[2], ==, 0.0f);
  munit_assert_float(get_glfloat(group->array)[3], ==, 0.0f);
  munit_assert_float(get_glfloat(group->array)[4], ==, 1.0f);
  munit_assert_float(get_glfloat(group->array)[5], ==, 0.0f);
  munit_assert_float(get_glfloat(group->array)[6], ==, 0.0f);
  munit_assert_float(get_glfloat(group->array)[7], ==, 1.0f);
  return MUNIT_OK;
}

static MunitResult test_material_filename(const MunitParameter params[], void *data)
{
  munit_assert_ptr(parse_string("mtllib empty.mtl\no test"), !=, NULL);
  return MUNIT_OK;
}

static MunitResult test_material_name(const MunitParameter params[], void *data)
{
  parse_string_core("mtllib test.mtl\no test");
  munit_assert_ptr(hash_find_material(parse_materials, "testmaterial", NULL), !=, NULL);
  return MUNIT_OK;
}

static MunitResult test_file_not_found(const MunitParameter params[], void *data)
{
  munit_assert_ptr(parse_string("mtllib nosuchfile.mtl\no test"), !=, NULL);
  return MUNIT_OK;
}

static MunitResult test_continue_after_include(const MunitParameter params[], void *data)
{
  parse_string_core("mtllib test.mtl\no test\nv 1 2 3");
  munit_assert_int(parse_vertex->size, ==, 3);
  return MUNIT_OK;
}

static MunitResult test_material(const MunitParameter params[], void *data)
{
  parse_string_core("newmtl stone\no test");
  material_t *stone = hash_find_material(parse_materials, "stone", NULL);
  munit_assert_ptr(stone, !=, NULL);
  return MUNIT_OK;
}

static MunitResult test_late_material(const MunitParameter params[], void *data)
{
  parse_string_core("o test\nnewmtl stone");
  material_t *stone = hash_find_material(parse_materials, "stone", NULL);
  munit_assert_ptr(stone, !=, NULL);
  return MUNIT_OK;
}

static MunitResult test_two_materials(const MunitParameter params[], void *data)
{
  parse_string_core("newmtl stone\nnewmtl water\no test");
  material_t *stone = hash_find_material(parse_materials, "stone", NULL);
  material_t *water = hash_find_material(parse_materials, "water", NULL);
  munit_assert_ptr(stone, !=, NULL);
  munit_assert_ptr(water, !=, NULL);
  return MUNIT_OK;
}

static MunitResult test_illumination_model(const MunitParameter params[], void *data)
{
  parse_string_core("newmtl test\nillum 6\no test");
  material_t *material = hash_find_material(parse_materials, "test", NULL);
  munit_assert_int(material->illumination, ==, 6);
  return MUNIT_OK;
}

static MunitResult test_ambient(const MunitParameter params[], void *data)
{
  parse_string_core("newmtl test\nKa 0.25 0.5 0.75\no test");
  material_t *material = hash_find_material(parse_materials, "test", NULL);
  munit_assert_float(material->ambient[0], ==, 0.25f);
  munit_assert_float(material->ambient[1], ==, 0.5f);
  munit_assert_float(material->ambient[2], ==, 0.75f);
  return MUNIT_OK;
}

static MunitResult test_use_material(const MunitParameter params[], void *data)
{
  object_t *object = parse_string("newmtl mat\no test\nusemtl mat\ng group");
  group_t *group = get_pointer(object->group)[0];
  munit_assert_ptr(group->material, !=, NULL);
  return MUNIT_OK;
}

static MunitResult test_reuse_material(const MunitParameter params[], void *data)
{
  object_t *object = parse_string("newmtl mat\no test\nusemtl mat\ng group\ng group2");
  group_t *group = get_pointer(object->group)[1];
  munit_assert_ptr(group->material, !=, NULL);
  return MUNIT_OK;
}

static MunitResult test_read_texture(const MunitParameter params[], void *data)
{
  parse_string_core("newmtl test\nmap_Kd colors.png\no test");
  material_t *material = hash_find_material(parse_materials, "test", NULL);
  munit_assert_ptr(material->diffuse_texture, !=, NULL);
  return MUNIT_OK;
}

static MunitResult test_texture_not_found(const MunitParameter params[], void *data)
{
  parse_string_core("newmtl test\nmap_Kd nosuchfile.png\no test");
  material_t *material = hash_find_material(parse_materials, "test", NULL);
  munit_assert_ptr(material->diffuse_texture, ==, NULL);
  return MUNIT_OK;
}

static MunitResult test_specular_texture(const MunitParameter params[], void *data)
{
  parse_string_core("newmtl test\nmap_Ks colors.png\no test");
  material_t *material = hash_find_material(parse_materials, "test", NULL);
  munit_assert_ptr(material->specular_texture, !=, NULL);
  return MUNIT_OK;
}

static MunitResult test_diffuse(const MunitParameter params[], void *data)
{
  parse_string_core("newmtl test\nKd 0.25 0.5 0.75\no test");
  material_t *material = hash_find_material(parse_materials, "test", NULL);
  munit_assert_float(material->diffuse[0], ==, 0.25f);
  munit_assert_float(material->diffuse[1], ==, 0.5f);
  munit_assert_float(material->diffuse[2], ==, 0.75f);
  return MUNIT_OK;
}

static MunitResult test_specular(const MunitParameter params[], void *data)
{
  parse_string_core("newmtl test\nKs 0.25 0.5 0.75\no test");
  material_t *material = hash_find_material(parse_materials, "test", NULL);
  munit_assert_float(material->specular[0], ==, 0.25f);
  munit_assert_float(material->specular[1], ==, 0.5f);
  munit_assert_float(material->specular[2], ==, 0.75f);
  return MUNIT_OK;
}

static MunitResult test_specular_exponent(const MunitParameter params[], void *data)
{
  parse_string_core("newmtl test\nNs 6.5\no test");
  material_t *material = hash_find_material(parse_materials, "test", NULL);
  munit_assert_float(material->specular_exponent, ==, 6.5f);
  return MUNIT_OK;
}

static MunitResult test_optical_density(const MunitParameter params[], void *data)
{
  parse_string_core("newmtl test\nNi 1.5\no test");
  material_t *material = hash_find_material(parse_materials, "test", NULL);
  munit_assert_float(material->optical_density, ==, 1.5f);
  return MUNIT_OK;
}

static MunitResult test_disolve(const MunitParameter params[], void *data)
{
  parse_string_core("newmtl test\nd 0.5\no test");
  material_t *material = hash_find_material(parse_materials, "test", NULL);
  munit_assert_float(material->disolve, ==, 0.5f);
  return MUNIT_OK;
}

static MunitResult test_mix_statements(const MunitParameter params[], void *data)
{
  object_t *object = parse_string("o test\nv 2 3 5\nv 3 5 7\nv 7 5 3\ng group\nf 1 2 3\nv 1 2 3");
  munit_assert_ptr(object, !=, NULL);
  return MUNIT_OK;
}

static MunitResult test_reset_parser(const MunitParameter params[], void *data)
{
  munit_assert_ptr(parse_string("o test"), !=, NULL);
  munit_assert_ptr(parse_string("o test"), !=, NULL);
  return MUNIT_OK;
}

MunitTest test_parser[] = {
  {"/empty"                  , test_empty                  , test_setup_gc, test_teardown_gc, MUNIT_TEST_OPTION_NONE, NULL},
  {"/object"                 , test_object                 , test_setup_gc, test_teardown_gc, MUNIT_TEST_OPTION_NONE, NULL},
  {"/object_name"            , test_object_name            , test_setup_gc, test_teardown_gc, MUNIT_TEST_OPTION_NONE, NULL},
  {"/ignore_whitespace"      , test_ignore_whitespace      , test_setup_gc, test_teardown_gc, MUNIT_TEST_OPTION_NONE, NULL},
  {"/whitespace_in_name"     , test_whitespace_in_name     , test_setup_gc, test_teardown_gc, MUNIT_TEST_OPTION_NONE, NULL},
  {"/handle_ctrl_lf"         , test_handle_ctrl_lf         , test_setup_gc, test_teardown_gc, MUNIT_TEST_OPTION_NONE, NULL},
  {"/error"                  , test_error                  , test_setup_gc, test_teardown_gc, MUNIT_TEST_OPTION_NONE, NULL},
  {"/read_file"              , test_read_file              , test_setup_gc, test_teardown_gc, MUNIT_TEST_OPTION_NONE, NULL},
  {"/no_such_file"           , test_no_such_file           , test_setup_gc, test_teardown_gc, MUNIT_TEST_OPTION_NONE, NULL},
  {"/ignore_comments"        , test_ignore_comments        , test_setup_gc, test_teardown_gc, MUNIT_TEST_OPTION_NONE, NULL},
  {"/ignore_trailing_comment", test_ignore_trailing_comment, test_setup_gc, test_teardown_gc, MUNIT_TEST_OPTION_NONE, NULL},
  {"/no_newline_in_name"     , test_no_newline_in_name     , test_setup_gc, test_teardown_gc, MUNIT_TEST_OPTION_NONE, NULL},
  {"/no_vertices"            , test_no_vertices            , test_setup_gc, test_teardown_gc, MUNIT_TEST_OPTION_NONE, NULL},
  {"/read_vertex"            , test_read_vertex            , test_setup_gc, test_teardown_gc, MUNIT_TEST_OPTION_NONE, NULL},
  {"/two_vertices"           , test_two_vertices           , test_setup_gc, test_teardown_gc, MUNIT_TEST_OPTION_NONE, NULL},
  {"/exponent"               , test_exponent               , test_setup_gc, test_teardown_gc, MUNIT_TEST_OPTION_NONE, NULL},
  {"/cleanup_vertices"       , test_cleanup_vertices       , test_setup_gc, test_teardown_gc, MUNIT_TEST_OPTION_NONE, NULL},
  {"/no_groups"              , test_no_groups              , test_setup_gc, test_teardown_gc, MUNIT_TEST_OPTION_NONE, NULL},
  {"/start_group"            , test_start_group            , test_setup_gc, test_teardown_gc, MUNIT_TEST_OPTION_NONE, NULL},
  {"/group_name"             , test_group_name             , test_setup_gc, test_teardown_gc, MUNIT_TEST_OPTION_NONE, NULL},
  {"/two_groups"             , test_two_groups             , test_setup_gc, test_teardown_gc, MUNIT_TEST_OPTION_NONE, NULL},
  {"/default_object"         , test_default_object         , test_setup_gc, test_teardown_gc, MUNIT_TEST_OPTION_NONE, NULL},
  {"/facet"                  , test_facet                  , test_setup_gc, test_teardown_gc, MUNIT_TEST_OPTION_NONE, NULL},
  {"/vertex_stride"          , test_vertex_stride          , test_setup_gc, test_teardown_gc, MUNIT_TEST_OPTION_NONE, NULL},
  {"/indices"                , test_indices                , test_setup_gc, test_teardown_gc, MUNIT_TEST_OPTION_NONE, NULL},
  {"/coord_count"            , test_coord_count            , test_setup_gc, test_teardown_gc, MUNIT_TEST_OPTION_NONE, NULL},
  {"/copy_coords"            , test_copy_coords            , test_setup_gc, test_teardown_gc, MUNIT_TEST_OPTION_NONE, NULL},
  {"/shuffle_coords"         , test_shuffle_coords         , test_setup_gc, test_teardown_gc, MUNIT_TEST_OPTION_NONE, NULL},
  {"/relative_index"         , test_relative_index         , test_setup_gc, test_teardown_gc, MUNIT_TEST_OPTION_NONE, NULL},
  {"/generate_indices"       , test_generate_indices       , test_setup_gc, test_teardown_gc, MUNIT_TEST_OPTION_NONE, NULL},
  {"/reset_vertex_cache"     , test_reset_vertex_cache     , test_setup_gc, test_teardown_gc, MUNIT_TEST_OPTION_NONE, NULL},
  {"/two_facets"             , test_two_facets             , test_setup_gc, test_teardown_gc, MUNIT_TEST_OPTION_NONE, NULL},
  {"/reuse_vertices"         , test_reuse_vertices         , test_setup_gc, test_teardown_gc, MUNIT_TEST_OPTION_NONE, NULL},
  {"/square_facet"           , test_square_facet           , test_setup_gc, test_teardown_gc, MUNIT_TEST_OPTION_NONE, NULL},
  {"/no_group"               , test_no_group               , test_setup_gc, test_teardown_gc, MUNIT_TEST_OPTION_NONE, NULL},
  {"/add_group"              , test_add_group              , test_setup_gc, test_teardown_gc, MUNIT_TEST_OPTION_NONE, NULL},
  {"/no_texcoord"            , test_no_texcoord            , test_setup_gc, test_teardown_gc, MUNIT_TEST_OPTION_NONE, NULL},
  {"/cleanup_texcoords"      , test_cleanup_texcoords      , test_setup_gc, test_teardown_gc, MUNIT_TEST_OPTION_NONE, NULL},
  {"/read_texcoord"          , test_read_texcoord          , test_setup_gc, test_teardown_gc, MUNIT_TEST_OPTION_NONE, NULL},
  {"/two_texcoords"          , test_two_texcoords          , test_setup_gc, test_teardown_gc, MUNIT_TEST_OPTION_NONE, NULL},
  {"/uv_facet"               , test_uv_facet               , test_setup_gc, test_teardown_gc, MUNIT_TEST_OPTION_NONE, NULL},
  {"/vertex_and_uv_stride"   , test_vertex_and_uv_stride   , test_setup_gc, test_teardown_gc, MUNIT_TEST_OPTION_NONE, NULL},
  {"/reuse_uv"               , test_reuse_uv               , test_setup_gc, test_teardown_gc, MUNIT_TEST_OPTION_NONE, NULL},
  {"/uv_indices"             , test_uv_indices             , test_setup_gc, test_teardown_gc, MUNIT_TEST_OPTION_NONE, NULL},
  {"/uv_added"               , test_uv_added               , test_setup_gc, test_teardown_gc, MUNIT_TEST_OPTION_NONE, NULL},
  {"/copy_uv"                , test_copy_uv                , test_setup_gc, test_teardown_gc, MUNIT_TEST_OPTION_NONE, NULL},
  {"/relative_uv_indices"    , test_relative_uv_indices    , test_setup_gc, test_teardown_gc, MUNIT_TEST_OPTION_NONE, NULL},
  {"/different_uv_index"     , test_different_uv_index     , test_setup_gc, test_teardown_gc, MUNIT_TEST_OPTION_NONE, NULL},
  {"/uv_in_hash_key"         , test_uv_in_hash_key         , test_setup_gc, test_teardown_gc, MUNIT_TEST_OPTION_NONE, NULL},
  {"/vertex_and_uv_key"      , test_vertex_and_uv_key      , test_setup_gc, test_teardown_gc, MUNIT_TEST_OPTION_NONE, NULL},
  {"/no_normal"              , test_no_normal              , test_setup_gc, test_teardown_gc, MUNIT_TEST_OPTION_NONE, NULL},
  {"/cleanup_normals"        , test_cleanup_normals        , test_setup_gc, test_teardown_gc, MUNIT_TEST_OPTION_NONE, NULL},
  {"/read_normal"            , test_read_normal            , test_setup_gc, test_teardown_gc, MUNIT_TEST_OPTION_NONE, NULL},
  {"/normal_facet"           , test_normal_facet           , test_setup_gc, test_teardown_gc, MUNIT_TEST_OPTION_NONE, NULL},
  {"/normal_indices"         , test_normal_indices         , test_setup_gc, test_teardown_gc, MUNIT_TEST_OPTION_NONE, NULL},
  {"/normals_added"          , test_normals_added          , test_setup_gc, test_teardown_gc, MUNIT_TEST_OPTION_NONE, NULL},
  {"/copy_normal"            , test_copy_normal            , test_setup_gc, test_teardown_gc, MUNIT_TEST_OPTION_NONE, NULL},
  {"/relative_normal_index"  , test_relative_normal_index  , test_setup_gc, test_teardown_gc, MUNIT_TEST_OPTION_NONE, NULL},
  {"/different_normal_index" , test_different_normal_index , test_setup_gc, test_teardown_gc, MUNIT_TEST_OPTION_NONE, NULL},
  {"/reuse_normal"           , test_reuse_normal           , test_setup_gc, test_teardown_gc, MUNIT_TEST_OPTION_NONE, NULL},
  {"/normal_in_hash_key"     , test_normal_in_hash_key     , test_setup_gc, test_teardown_gc, MUNIT_TEST_OPTION_NONE, NULL},
  {"/vertex_and_normal_key"  , test_vertex_and_normal_key  , test_setup_gc, test_teardown_gc, MUNIT_TEST_OPTION_NONE, NULL},
  {"/full_facet"             , test_full_facet             , test_setup_gc, test_teardown_gc, MUNIT_TEST_OPTION_NONE, NULL},
  {"/full_indices"           , test_full_indices           , test_setup_gc, test_teardown_gc, MUNIT_TEST_OPTION_NONE, NULL},
  {"/all_added"              , test_all_added              , test_setup_gc, test_teardown_gc, MUNIT_TEST_OPTION_NONE, NULL},
  {"/different_indices"      , test_different_indices      , test_setup_gc, test_teardown_gc, MUNIT_TEST_OPTION_NONE, NULL},
  {"/material_filename"      , test_material_filename      , test_setup_gc, test_teardown_gc, MUNIT_TEST_OPTION_NONE, NULL},
  {"/material_name"          , test_material_name          , test_setup_gc, test_teardown_gc, MUNIT_TEST_OPTION_NONE, NULL},
  {"/file_not_found"         , test_file_not_found         , test_setup_gc, test_teardown_gc, MUNIT_TEST_OPTION_NONE, NULL},
  {"/continue_after_include" , test_continue_after_include , test_setup_gc, test_teardown_gc, MUNIT_TEST_OPTION_NONE, NULL},
  {"/material"               , test_material               , test_setup_gc, test_teardown_gc, MUNIT_TEST_OPTION_NONE, NULL},
  {"/late_material"          , test_late_material          , test_setup_gc, test_teardown_gc, MUNIT_TEST_OPTION_NONE, NULL},
  {"/two_materials"          , test_two_materials          , test_setup_gc, test_teardown_gc, MUNIT_TEST_OPTION_NONE, NULL},
  {"/illumination_model"     , test_illumination_model     , test_setup_gc, test_teardown_gc, MUNIT_TEST_OPTION_NONE, NULL},
  {"/ambient"                , test_ambient                , test_setup_gc, test_teardown_gc, MUNIT_TEST_OPTION_NONE, NULL},
  {"/use_material"           , test_use_material           , test_setup_gc, test_teardown_gc, MUNIT_TEST_OPTION_NONE, NULL},
  {"/reuse_material"         , test_reuse_material         , test_setup_gc, test_teardown_gc, MUNIT_TEST_OPTION_NONE, NULL},
  {"/read_texture"           , test_read_texture           , test_setup_gl, test_teardown_gl, MUNIT_TEST_OPTION_NONE, NULL},
  {"/texture_not_found"      , test_texture_not_found      , test_setup_gc, test_teardown_gc, MUNIT_TEST_OPTION_NONE, NULL},
  {"/specular_texture"       , test_specular_texture       , test_setup_gl, test_teardown_gl, MUNIT_TEST_OPTION_NONE, NULL},
  {"/diffuse"                , test_diffuse                , test_setup_gc, test_teardown_gc, MUNIT_TEST_OPTION_NONE, NULL},
  {"/specular"               , test_specular               , test_setup_gc, test_teardown_gc, MUNIT_TEST_OPTION_NONE, NULL},
  {"/specular_exponent"      , test_specular_exponent      , test_setup_gc, test_teardown_gc, MUNIT_TEST_OPTION_NONE, NULL},
  {"/optical_density"        , test_optical_density        , test_setup_gc, test_teardown_gc, MUNIT_TEST_OPTION_NONE, NULL},
  {"/disolve"                , test_disolve                , test_setup_gc, test_teardown_gc, MUNIT_TEST_OPTION_NONE, NULL},
  {"/mix_statements"         , test_mix_statements         , test_setup_gc, test_teardown_gc, MUNIT_TEST_OPTION_NONE, NULL},
  {"/reset_parser"           , test_reset_parser           , test_setup_gc, test_teardown_gc, MUNIT_TEST_OPTION_NONE, NULL},
  {NULL                      , NULL                        , NULL         , NULL            , MUNIT_TEST_OPTION_NONE, NULL}
};
