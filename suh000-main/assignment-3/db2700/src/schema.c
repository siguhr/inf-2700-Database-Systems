/***********************************************************
 * Schema for assignments in the Databases course INF-2700 *
 * UIT - The Arctic University of Norway                   *
 * Author: Weihai Yu                                       *
 ************************************************************/

#include "schema.h"
#include "pmsg.h"
#include <string.h>

/** @brief Field descriptor */
typedef struct field_desc_struct {
  char *name;        /**< field name */
  field_type type;   /**< field type */
  int len;           /**< field length (number of bytes) */
  int offset;        /**< offset from the beginning of the record */
  field_desc_p next; /**< next field_desc of the table, NULL if no more */
} field_desc_struct;

/** @brief Table/record schema */
/** A schema is a linked list of @ref field_desc_struct "field descriptors".
    All records of a table are of the same length.
*/
typedef struct schema_struct {
  char *name;           /**< schema (table) name */
  field_desc_p first;   /**< first field_desc */
  field_desc_p last;    /**< last field_desc */
  int num_fields;       /**< number of fields in the table */
  int len;              /**< record length */
  tbl_p tbl;            /**< table descriptor */
} schema_struct;

/** @brief Table descriptor */
/** A table descriptor allows us to find the schema and
    run-time infomation about the table.
 */
typedef struct tbl_desc_struct {
  schema_p sch;      /**< schema of this table. */
  int num_records;   /**< number of records this table has. */
  page_p current_pg; /**< current page being accessed. */
  tbl_p next;        /**< next tbl_desc in the database. */
} tbl_desc_struct;


/** @brief Database tables*/
tbl_p db_tables; /**< a linked list of table descriptors */

void put_field_info(pmsg_level level, field_desc_p f) {
  if (!f) {
    put_msg(level,  "  empty field\n");
    return;
  }
  put_msg(level, "  \"%s\", ", f->name);
  if (is_int_field(f))
    append_msg(level,  "int ");
  else
    append_msg(level,  "str ");
  append_msg(level, "field, len: %d, offset: %d, ", f->len, f->offset);
  if (f->next)
    append_msg(level,  ", next field: %s\n", f->next->name);
  else
    append_msg(level,  "\n");
}

void put_schema_info(pmsg_level level, schema_p s) {
  if (!s) {
    put_msg(level,  "--empty schema\n");
    return;
  }
  field_desc_p f;
  put_msg(level, "--schema %s: %d field(s), totally %d bytes\n",
          s->name, s->num_fields, s->len);
  for (f = s->first; f; f = f->next)
    put_field_info(level, f);
  put_msg(level, "--\n");
}

void put_tbl_info(pmsg_level level, tbl_p t) {
  if (!t) {
    put_msg(level,  "--empty tbl desc\n");
    return;
  }
  put_schema_info(level, t->sch);
  put_file_info(level, t->sch->name);
  put_msg(level, " %d blocks, %d records\n",
          file_num_blocks(t->sch->name), t->num_records);
  put_msg(level, "----\n");
}

void put_record_info(pmsg_level level, record r, schema_p s) {
  field_desc_p f;
  size_t i = 0;
  put_msg(level, "Record: ");
  for (f = s->first; f; f = f->next, i++) {
    if (is_int_field(f))
      append_msg(level,  "%d", *(int *)r[i]);
    else
      append_msg(level,  "%s", (char *)r[i]);

    if (f->next)
      append_msg(level,  " | ");
  }
  append_msg(level,  "\n");
}

void put_db_info(pmsg_level level) {
  char *db_dir = system_dir();
  if (!db_dir) return;
  put_msg(level, "======Database at %s:\n", db_dir);
  for (tbl_p tbl = db_tables; tbl; tbl = tbl->next)
    put_tbl_info(level, tbl);
  put_msg(level, "======\n");
}

field_desc_p new_int_field(char const* name) {
  field_desc_p res = malloc(sizeof (field_desc_struct));
  res->name = strdup(name);
  res->type = INT_TYPE;
  res->len = INT_SIZE;
  res->offset = 0;
  res->next = 0;
  return res;
}

field_desc_p new_str_field(char const* name, int len) {
  field_desc_p res = malloc(sizeof (field_desc_struct));
  res->name = strdup(name);
  res->type = STR_TYPE;
  res->len = len;
  res->offset = 0;
  res->next = 0;
  return res;
}

static void release_field_desc(field_desc_p f) {
  if (f) {
    free(f->name);
    free(f);
    f = 0;
  }
}

int is_int_field(field_desc_p f) {
  return f ? (f->type == INT_TYPE) : 0;
}

field_desc_p field_desc_next(field_desc_p f) {
  if (f)
    return f->next;
  else {
    put_msg(ERROR, "field_desc_next: NULL field_desc_next.\n");
    return 0;
  }
}

static schema_p make_schema(char const* name) {
  schema_p res = malloc(sizeof (schema_struct));
  res->name = strdup(name);
  res->first = 0;
  res->last = 0;
  res->num_fields = 0;
  res->len = 0;
  return res;
}

/** Release the memory allocated for the schema and its field descriptors.*/
static void release_schema(schema_p sch) {
  if (!sch) return;

  field_desc_p f, nextf;
  f = sch->first;
  while (f) {
    nextf = f->next;
    release_field_desc(f);
    f = nextf;
  }
  free(sch->name);
  free(sch);
}

char const* const schema_name(schema_p sch) {
  if (sch)
    return sch->name;
  else {
    put_msg(ERROR, "schema_name: NULL schema.\n");
    return 0;
  }
}

field_desc_p schema_first_fld_desc(schema_p sch) {
  if (sch)
    return sch->first;
  else {
    put_msg(ERROR, "schema_first_fld_desc: NULL schema.\n");
    return 0;
  }
}

field_desc_p schema_last_fld_desc(schema_p sch) {
  if (sch)
    return sch->last;
  else {
    put_msg(ERROR, "schema_last_fld_desc: NULL schema.\n");
    return 0;
  }
}

int schema_num_flds(schema_p sch) {
  if (sch)
    return sch->num_fields;
  else {
    put_msg(ERROR, "schema_num_flds: NULL schema.\n");
    return -1;
  }
}

int schema_len(schema_p sch) {
  if (sch)
    return sch->len;
  else {
    put_msg(ERROR, "schema_len: NULL schema.\n");
    return -1;
  }
}

const char tables_desc_file[] = "db.db"; /***< File holding table descriptors */

static char* concat_names(char const* name1, char const* sep, char const* name2) {
  char *res = malloc(strlen(name1) + strlen(sep) + strlen(name2) + 1);
  strcpy(res, name1);
  strcat(res, sep);
  strcat(res, name2);
  return res;
}

static void save_tbl_desc(FILE *fp, tbl_p tbl) {
  schema_p sch = tbl->sch;
  fprintf(fp, "%s %d\n", sch->name, sch->num_fields);
  field_desc_p fld = schema_first_fld_desc(sch);
  while (fld) {
    fprintf(fp, "%s %d %d %d\n",
            fld->name, fld->type, fld->len, fld->offset);
    fld = fld->next;
  }
  fprintf(fp, "%d\n", tbl->num_records);
}

static void save_tbl_descs() {
  /* backup the descriptors first in case we need some manual investigation */
  char *tbl_desc_backup = concat_names("__backup", "_", tables_desc_file);
  rename(tables_desc_file, tbl_desc_backup);
  free(tbl_desc_backup);

  FILE *dbfile = fopen(tables_desc_file, "w");
  tbl_p tbl = db_tables, next_tbl = 0;
  while (tbl) {
    save_tbl_desc(dbfile, tbl);
    release_schema(tbl->sch);
    next_tbl = tbl->next;
    free(tbl);
    tbl = next_tbl;
  }
  fclose(dbfile);
}

static void read_tbl_descs() {
  FILE *fp = fopen(tables_desc_file, "r");
  if (!fp) return;
  char name[30] = "";
  schema_p sch = NULL;
  field_desc_p fld = NULL;
  int num_flds = 0, fld_type, fld_len;
  while (!feof(fp)) {
    if (fscanf(fp, "%s %d\n", name, &num_flds) < 2) {
      fclose(fp);
      return;
    }
    sch = new_schema(name);
    for (size_t i = 0; i < num_flds; i++) {
      fscanf(fp, "%s %d %d", name, &(fld_type), &(fld_len));
      switch (fld_type) {
      case INT_TYPE:
        fld = new_int_field(name);
        break;
      case STR_TYPE:
        fld = new_str_field(name, fld_len);
        break;
      }
      fscanf(fp, "%d\n", &(fld->offset));
      add_field(sch, fld);
    }
    fscanf(fp, "%d\n", &(sch->tbl->num_records));
  }
  db_tables = sch->tbl;
  fclose(fp);
}

int open_db(void) {
  pager_terminate(); /* first clean up for a fresh start */
  pager_init();
  read_tbl_descs();
  return 1;
}

void close_db(void) {
  save_tbl_descs();
  db_tables = 0;
  pager_terminate();
}

schema_p new_schema(char const* name) {
  tbl_p tbl = malloc(sizeof (tbl_desc_struct));
  tbl->sch = make_schema(name);
  tbl->sch->tbl = tbl;
  tbl->num_records = 0;
  tbl->current_pg = 0;
  tbl->next = db_tables;
  db_tables = tbl;
  return tbl->sch;
}

tbl_p get_table(char const* name) {
  for (tbl_p tbl = db_tables; tbl; tbl = tbl->next)
    if (strcmp(name, tbl->sch->name) == 0)
      return tbl;
  return 0;
}

schema_p get_schema(char const* name) {
  tbl_p tbl = get_table(name);
  if (tbl) return tbl->sch;
  else return 0;
}

void remove_table(tbl_p t) {
  if (!t) return;

  for (tbl_p tbl = db_tables, prev = 0;
       tbl;
       prev = tbl, tbl = tbl->next)
    if (tbl == t) {
      if (t == db_tables)
        db_tables = t->next;
      else
        prev->next = t->next;

      close_file(t->sch->name);
      char *tbl_backup = concat_names("_", "_", t->sch->name);
      rename(t->sch->name, tbl_backup);
      free(tbl_backup);
      release_schema(t->sch);
      free(t);
      return;
    }
}

void remove_schema(schema_p s) {
  if (s) remove_table(s->tbl);
}

static field_desc_p dup_field(field_desc_p f) {
  field_desc_p res = malloc(sizeof (field_desc_struct));
  res->name = strdup(f->name);
  res->type = f->type;
  res->len = f->len;
  res->offset = 0;
  res->next = 0;
  return res;
}

static schema_p copy_schema(schema_p s, char const* dest_name) {
  if (!s) return 0;
  schema_p dest = new_schema(dest_name);
  for (field_desc_p f = s->first; f; f = f->next)
    add_field(dest, dup_field(f));
  return dest;
}

static field_desc_p get_field(schema_p s, char const* name) {
  for (field_desc_p f = s->first; f; f = f->next)
    if (strcmp(f->name, name) == 0) return f;
  return 0;
}

static char* tmp_schema_name(char const* op_name, char const* name) {
  char *res = malloc(strlen(op_name) + strlen(name) + 10);
  int i = 0;
  do
    sprintf(res, "%s__%s_%d", op_name, name, i++);
  while (get_schema(res));

  return res;
}

static schema_p make_sub_schema(schema_p s, int num_fields, char *fields[]) {
  if (!s) return 0;

  char *sub_sch_name = tmp_schema_name("project", s->name);
  schema_p res = new_schema(sub_sch_name);
  free(sub_sch_name);
  
  field_desc_p f = 0;
  for (size_t i= 0; i < num_fields; i++) {
    f = get_field(s, fields[i]);
    if (f)
      add_field(res, dup_field(f));
    else {
      put_msg(ERROR, "\"%s\" has no \"%s\" field\n",
              s->name, fields[i]);
      remove_schema(res);
      return 0;
    }
  }
  return res;
}

int add_field(schema_p s, field_desc_p f) {
  if (!s) return 0;
  if (s->len + f->len > BLOCK_SIZE - PAGE_HEADER_SIZE) {
    put_msg(ERROR,
            "schema already has %d bytes, adding %d will exceed limited %d bytes.\n",
            s->len, f->len, BLOCK_SIZE - PAGE_HEADER_SIZE);
    return 0;
  }
  if (s->num_fields == 0) {
    s->first = f;
    f->offset = 0;
  }
  else {
    s->last->next = f;
    f->offset = s->len;
  }
  s->last = f;
  s->num_fields++;
  s->len += f->len;
  return s->num_fields;
}

record new_record(schema_p s) {
  if (!s) {
    put_msg(ERROR,  "new_record: NULL schema!\n");
    exit(EXIT_FAILURE);
  }
  record res = malloc((sizeof (void *)) * s->num_fields);

  /* allocate memory for the fields */
  field_desc_p f;
  size_t i = 0;
  for (f = s->first; f; f = f->next, i++) {
    res[i] =  malloc(f->len);
  }
  return res;
}

void release_record(record r, schema_p s) {
  if (!(r && s)) {
    put_msg(ERROR,  "release_record: NULL record or schema!\n");
    return;
  }
  for (size_t i = 0; i < s->num_fields; i++)
    free(r[i]);
  free(r);
  r = 0;
}

void assign_int_field(void const* field_p, int int_val) {
  *(int *)field_p = int_val;
}

void assign_str_field(void* field_p, char const* str_val) {
  strcpy(field_p, str_val);
}

int fill_record(record r, schema_p s, ...) {
  if (!(r && s)) {
    put_msg(ERROR,  "fill_record: NULL record or schema!\n");
    return 0;
  }
  va_list vals;
  va_start(vals, s);
  field_desc_p f;
  size_t i = 0;
  for (f = s->first; f; f = f->next, i++) {
    if (is_int_field(f))
      assign_int_field(r[i], va_arg(vals, int));
    else
      assign_str_field(r[i], va_arg(vals, char*));
  }
  return 1;
}

static void fill_sub_record(record dest_r, schema_p dest_s,
                            record src_r, schema_p src_s) {
  field_desc_p src_f, dest_f;
  size_t i = 0, j = 0;
  for (dest_f = dest_s->first; dest_f; dest_f = dest_f->next, i++) {
    for (j = 0, src_f = src_s->first;
         strcmp(src_f->name, dest_f->name) != 0;
         j++, src_f = src_f->next)
      ;
    if (is_int_field(dest_f))
      assign_int_field(dest_r[i], *(int *)src_r[j]);
    else
      assign_str_field(dest_r[i], (char *)src_r[j]);
  }
}

int equal_record(record r1, record r2, schema_p s) {
  if (!(r1 && r2 && s)) {
    put_msg(ERROR,  "equal_record: NULL record or schema!\n");
    return 0;
  }

  field_desc_p fd;
  size_t i = 0;;
  for (fd = s->first; fd; fd = fd->next, i++) {
    if (is_int_field(fd)) {
      if (*(int *)r1[i] != *(int *)r2[i])
        return 0;
    }
    else {
      if (strcmp((char *)r1[i], (char *)r2[i]) != 0)
        return 0;
    }
  }
  return 1;
}

void set_tbl_position(tbl_p t, tbl_position pos) {
  switch (pos) {
  case TBL_BEG:
    {
      t->current_pg = get_page(t->sch->name, 0);
      page_set_pos_begin(t-> current_pg);
    }
    break;
  case TBL_END:
    t->current_pg = get_page_for_append(t->sch->name);
  }
}

int eot(tbl_p t) {
  return (peof(t->current_pg));
}

/** check if the the current position is valid */
static int page_valid_pos_for_get_with_schema(page_p p, schema_p s) {
  return (page_valid_pos_for_get(p, page_current_pos(p))
          && (page_current_pos(p) - PAGE_HEADER_SIZE) % s->len == 0);
}

/** check if the the current position is valid */
static int page_valid_pos_for_put_with_schema(page_p p, schema_p s) {
  return (page_valid_pos_for_put(p, page_current_pos(p), s->len)
          && (page_current_pos(p) - PAGE_HEADER_SIZE) % s->len == 0);
}

static page_p get_page_for_next_record(schema_p s) {
  page_p pg = s->tbl->current_pg;
  if (peof(pg)) return 0;
  if (eop(pg)) {
    unpin(pg);
    pg = get_next_page(pg);
    if (!pg) {
      put_msg(FATAL, "get_page_for_next_record failed at block %d\n",
              page_block_nr(pg) + 1);
      exit(EXIT_FAILURE);
    }
    page_set_pos_begin(pg);
    s->tbl->current_pg = pg;
  }
  return pg;
}

static int get_page_record(page_p p, record r, schema_p s) {
  if (!p) return 0;
  if (!page_valid_pos_for_get_with_schema(p, s)) {
    put_msg(FATAL, "try to get record at invalid position.\n");
    exit(EXIT_FAILURE);
  }
  field_desc_p fld_desc;
  size_t i = 0;
  for (fld_desc = s->first; fld_desc;
       fld_desc = fld_desc->next, i++)
    if (is_int_field(fld_desc))
      assign_int_field(r[i], page_get_int(p));
    else
      page_get_str(p, r[i], fld_desc->len);
  return 1;
}

int get_record(record r, schema_p s) {
  page_p pg = get_page_for_next_record(s);
  return pg ? get_page_record(pg, r, s) : 0;
}
/*Relational operators*/
static int int_equal(int x, int y) 
{
  return x == y;
}


static int int_is_more(int x, int y)
{
  return x > y;
}

static int int_is_more_or_equal(int x, int y)
{
  return x >= y;
}
static int int_is_less(int x, int y)
{
  return x < y;
}
static int int_is_less_or_equal(int x, int y)
{
  return x <= y;
}
static int int_is_not_equal(int x, int y)
{
  return x != y;
}
/*Binar logic*/
static int bi_int_equal(int x, int y) 
{
  return x == y;
}


static int find_record_int_val(record r, schema_p s, int offset,
                               int (*op) (int, int), int val) {
  page_p pg = get_page_for_next_record(s);
  if (!pg) return 0;
  int pos, rec_val;
  for (; pg; pg = get_page_for_next_record(s)) {
    pos = page_current_pos(pg);
    rec_val = page_get_int_at (pg, pos + offset);
    if ((*op) (val, rec_val)) {
      page_set_current_pos(pg, pos);
      get_page_record(pg, r, s);
      return 1;
    }
    else
      page_set_current_pos(pg, pos + s->len);
  }
  return 0;
}

static int put_page_record(page_p p, record r, schema_p s) {
  if (!page_valid_pos_for_put_with_schema(p, s))
    return 0;

  field_desc_p fld_desc;
  size_t i = 0;
  for (fld_desc = s->first; fld_desc;
       fld_desc = fld_desc->next, i++)
    if (is_int_field(fld_desc))
      page_put_int(p, *(int *)r[i]);
    else
      page_put_str(p, (char *)r[i], fld_desc->len);
  return 1;
}

int put_record(record r, schema_p s) {
  page_p p = s->tbl->current_pg;

  if (!page_valid_pos_for_put_with_schema(p, s))
    return 0;

  field_desc_p fld_desc;
  size_t i = 0;
  for (fld_desc = s->first; fld_desc;
       fld_desc = fld_desc->next, i++)
    if (is_int_field(fld_desc))
      page_put_int(p, *(int *)r[i]);
    else
      page_put_str(p, (char *)r[i], fld_desc->len);
  return 1;
}

void append_record(record r, schema_p s) {
  tbl_p tbl = s->tbl;
  page_p pg = get_page_for_append(s->name);
  if (!pg) {
    put_msg(FATAL, "Failed to get page for appending to \"%s\".\n",
            s->name);
    exit(EXIT_FAILURE);
  }
  if (!put_page_record(pg, r, s)) {
    /* not enough space in the current page */
    unpin(pg);
    pg = get_next_page(pg);
    if (!pg) {
      put_msg(FATAL, "Failed to get page for \"%s\" block %d.\n",
              s->name, page_block_nr(pg) + 1);
      exit(EXIT_FAILURE);
    }
    if (!put_page_record(pg, r, s)) {
      put_msg(FATAL, "Failed to put record to page for \"%s\" block %d.\n",
              s->name, page_block_nr(pg) + 1);
      exit(EXIT_FAILURE);
    }
  }
  tbl->current_pg = pg;
  tbl->num_records++;
}

static void display_tbl_header(tbl_p t) {
  if (!t) {
    put_msg(INFO,  "Trying to display non-existant table.\n");
    return;
  }
  schema_p s = t->sch;
  for (field_desc_p f = s->first; f; f = f->next)
    put_msg(FORCE, "%20s", f->name);
  put_msg(FORCE, "\n");
  for (field_desc_p f = s->first; f; f = f->next) {
    for (size_t i = 0; i < 20 - strlen(f->name); i++)
      put_msg(FORCE, " ");
    for (size_t i = 0; i < strlen(f->name); i++)
      put_msg(FORCE, "-");
  }
  put_msg(FORCE, "\n");
}

static void display_record(record r, schema_p s) {
  field_desc_p f = s->first;
  for (size_t i = 0; f; f = f->next, i++) {
    if (is_int_field(f))
      put_msg(FORCE, "%20d", *(int *)r[i]);
    else
      put_msg(FORCE, "%20s", (char *)r[i]);
  }
  put_msg(FORCE, "\n");
}

void table_display(tbl_p t) {
  if (!t) return;
  display_tbl_header(t);

  schema_p s = t->sch;
  record rec = new_record(s);
  set_tbl_position(t, TBL_BEG);
  while (get_record(rec, s)) {
    display_record(rec, s);
  }
  put_msg(FORCE, "\n");

  release_record(rec, s);
}

int binary_search(record const r, schema_p const s, int offset, int val)
{
  /* find lower min and upper max  blocks to calculate mid */
  int min = 0;                                      /*set min to zero*/
  int max = (s->tbl->num_records - 1) * s->len;     /*find upper limit*/
  int mid = (max + min) / 2;                        /*find mid*/
  mid -= (mid % s->len);                            /*find true mid*/

  /* find block number, 
  depending on the record length, we need to fit whole records in each block */
  int blk_size = BLOCK_SIZE - PAGE_HEADER_SIZE;     /*find actual block size- page header is 20 bytes */
  int free_bytes = blk_size - (blk_size % s->len);  /*find free bytes*/
  int blk_num = mid / free_bytes;                   /*use mid and free bytes to find block number */

  /* find offset to middle record */
  int rec_page_offset = mid % free_bytes;

  /* get page from block*/
  page_p mid_page = get_page(s->name, blk_num);
  if (!mid_page) {
    return 0;
  }

  while (min <= max) {

    int pos = rec_page_offset + PAGE_HEADER_SIZE;
    /* fetch stored value */
    int rec_val = page_get_int_at(mid_page, (pos + offset));

    if (rec_val < val)        /* if queried value is higher tan recorded value*/
    {      
      min = mid + s->len;     /*update min to be mid + 1(s->len)*/
      mid = (max + min) / 2;  /*calculate new mid */   
      mid -= (mid % s->len);  /*find true new mid */
    } 
    else if (rec_val > val)    /* if queried value is lower tan recorded value*/
    {
      max = mid - s->len;     /* update max to be mid - 1(s->len) */
      mid = (max + min) / 2;  /*calculate new mid*/
      mid -= (mid % s->len);  /*find true new mid */
    } 
    else if (rec_val == val)  /*recoded value equals queried value*/
    {
      page_set_current_pos(mid_page, pos);
      get_page_record(mid_page, r, s);
      return 1;
    }
    /* update offset, block_num and page from the new middle value */
    rec_page_offset = (mid % free_bytes);
    blk_num = mid / free_bytes;
    mid_page = get_page(s->name, blk_num);
  }
  return 0;
}


/* Relational Operators  */
tbl_p table_search(tbl_p t, char const* attr, char const* op, int val) 
{
  if (!t) return 0;

  int (*cmp_op)() = 0;

  if (strcmp(op, "=") == 0)
  {
    cmp_op = int_equal;
  }
  else if (strcmp(op, "<") == 0)
  {
    cmp_op = int_is_more;
  }
  else if (strcmp(op, "<=") == 0)
  {
    cmp_op = int_is_more_or_equal;
  }
  else if (strcmp(op, ">") == 0)
  {
    cmp_op = int_is_less;
  }
  else if (strcmp(op, ">=") == 0)
  {
    cmp_op = int_is_less_or_equal;
  }
  else if (strcmp(op, "!=") == 0)
  {
    cmp_op = int_is_not_equal;
  }
  /*Binary reserved operators*/
  if (strcmp(op, "==") == 0)
  {
    cmp_op = bi_int_equal;
  }

 

  if (!cmp_op) {
    put_msg(ERROR, "unknown comparison operator \"%s\".\n", op);
    return 0;
  }


  schema_p s = t->sch;
  field_desc_p f;
  size_t i = 0;
  for (f = s->first; f; f = f->next, i++)
    if (strcmp(f->name, attr) == 0) {
      if (f->type != INT_TYPE) {
        put_msg(ERROR, "\"%s\" is not an integer field.\n", attr);
        return 0;
      }
      break;
    }
  if (!f) return 0;

 
  // schema_p res_sch = copy_schema(s, tmp_name);
  // free(tmp_name);

  char tmp_name[30] = "tmp_tbl__";
  strcat(tmp_name, s->name);
  schema_p res_sch = copy_schema(s, tmp_name);

  record rec = new_record(s);


  
  set_tbl_position(t, TBL_BEG);

  /* Binary search to equality, need to use == to test binary */
  if (strcmp(op, "==") == 0) 
  {
    if (binary_search(rec, s, f->offset, val) == 1) 
    {
      put_record_info(DEBUG, rec, s);
      append_record(rec, res_sch);
    }
  } 
  else 
  {   
      while (find_record_int_val(rec, s, f->offset, cmp_op, val)) {
        put_record_info(DEBUG, rec, s);
        append_record(rec, res_sch);
      }
  }

  release_record(rec, s);
  put_pager_profiler_info(INFO);
  pager_profiler_reset();

  return res_sch->tbl;
}
  


tbl_p table_project(tbl_p t, int num_fields, char* fields[]) 
{
  schema_p s = t->sch;
  schema_p dest = make_sub_schema(s, num_fields, fields);
  if (!dest) return 0;

  record rec = new_record(s), rec_dest = new_record(dest);

  set_tbl_position(t, TBL_BEG);
  while (get_record(rec, s)) {
    fill_sub_record(rec_dest, dest, rec, s);
    put_record_info(DEBUG, rec_dest, dest);
    append_record(rec_dest, dest);
  }

  release_record(rec, s);
  release_record(rec_dest, dest);

  return dest->tbl;
}


//######################

tbl_p table_natural_join(tbl_p left, tbl_p right) {
  if (!(left && right)) 
  {
    put_msg(ERROR, "no table found!\n");
    return 0;
  }

  tbl_p res = 0;

  /* TODO: assignment 3 */
  schema_p left_search = left->sch;
  schema_p right_search = right->sch;

  schema_p result;
  tbl_p ret;
  /* iterate and find common field */
  for (field_desc_p fld = left_search->first; fld; fld = fld->next)
  {
    for(field_desc_p fld2 = right_search->first; fld2; fld2 = fld2->next)
    {
      if(strcmp(fld->name, fld2->name) == 0) 
      {
        result = join_schema(left_search, right_search, "tmp_sch");

        ret = nested_loop_join(left_search, right_search, result, fld, fld2);
        //ret = block_nested_loop_join(left_search, right_search, result, fld, fld2);
      }
    }
  }

  put_pager_profiler_info(INFO);
  return ret;
}

tbl_p nested_loop_join(schema_p left_search, schema_p right_search, schema_p dest, field_desc_p fld, field_desc_p fld2)
{
  record left_record = new_record(left_search);
  record right_record = new_record(right_search);
  record rec_dest = new_record(dest);

  set_tbl_position(left_search->tbl, TBL_BEG);
  set_tbl_position(right_search->tbl, TBL_BEG);

  int rec_val, rec_val2;
  /* Iterate left - outer relation*/
  for(page_p page_l = get_page_for_next_record(left_search); page_l; page_l = get_page_for_next_record(left_search)) 
  {
    int pg_pos = page_current_pos(page_l);
    get_page_record(page_l, left_record, left_search);

    rec_val = page_get_int_at(page_l, (pg_pos + fld->offset));
    set_tbl_position(right_search->tbl, TBL_BEG);
    
    /* iterate right - inner relation */
    for (page_p page_r = get_page_for_next_record(right_search); page_r; page_r = get_page_for_next_record(right_search))
    {
      int pg2_pos = page_current_pos(page_r);
      get_page_record(page_r, right_record, right_search);
      rec_val2 = page_get_int_at(page_r, (pg2_pos + fld2->offset));

      /* if statment for equal values in records. If true, join those records and append our new table */
      if (rec_val == rec_val2) 
      {
        join_records(rec_dest, dest, left_record, left_search, right_record, right_search);
        append_record(rec_dest, dest);
      }
      /* Update position */
      page_set_current_pos(page_r, (pg2_pos + right_search->len));
    }
    page_set_current_pos(page_l, (pg_pos + left_search->len));
  }
  release_record(rec_dest, dest);
  return dest->tbl;
}

tbl_p block_nested_loop_join(schema_p left_search, schema_p right_search, schema_p dest, field_desc_p fld, field_desc_p fld2) 
{
  /* Depending on search length, calculate bytes for records */
  int blk_size = BLOCK_SIZE - PAGE_HEADER_SIZE; 
  int free_bytes_left = blk_size - (blk_size % left_search->len);
  int free_bytes_right = blk_size - (blk_size % right_search->len);

  /* Find records per block */
  int RPB_s1 = free_bytes_left / left_search->len;
  int RPB_s2 = free_bytes_right / right_search->len;

  /* Blocks per table */
  int n_blocks_left = left_search->tbl->num_records / RPB_s1;
  int n_blocks_right = right_search->tbl->num_records / RPB_s2;

  record left_record = new_record(left_search);
  record right_record = new_record(right_search);
  record rec_dest = new_record(dest);

  int rec_val, rec_val2, pos, pos2;
  set_tbl_position(left_search->tbl, TBL_BEG);

  /* Iterate left*/
  for (int i = 0; i <= n_blocks_left; i++)
  {
    page_p blk_outer = get_page(left_search->name, i);
    set_tbl_position(right_search->tbl, TBL_BEG);
    
    /* Iterate outer block*/
    for (int j = 0; j <= n_blocks_right; j++)
    {
      page_p blk_inner = get_page(right_search->name, j);

      /* iterate inner block */
      for (int x = 0; x < RPB_s1; x++) 
      {
        /* get page again, in case its not in memory */
        blk_outer = get_page(left_search->name, i);
        /* manually set positions to read at */
        if (x == 0) 
        {
          pos = PAGE_HEADER_SIZE;
        } else 
        {
          pos = PAGE_HEADER_SIZE + (x * left_search->len);
        }

        page_set_current_pos(blk_outer, pos);
        if (peof(blk_outer))
        {
          break;
        }
        /* if iterations reaches end of block table, we're done in this inner block  */
        if (eot(left_search->tbl)) 
        {
          break;
        }
        get_record(left_record, left_search);
        rec_val = page_get_int_at(blk_outer, (pos + fld->offset));
        
        /* iterate records in inner block */
        for (int y = 0; y < RPB_s2; y++) 
        {
          blk_inner = get_page(right_search->name, j);
          if (y == 0) 
          {
            pos2 = PAGE_HEADER_SIZE;
          } else 
          {
            pos2 = PAGE_HEADER_SIZE + (y * right_search->len);
          }
          page_set_current_pos(blk_inner, pos2);
          if (peof(blk_inner)) {
            break;
          }
          get_record(right_record, right_search);
          rec_val2 = page_get_int_at(blk_inner, (pos2 + fld2->offset));
          
          if (rec_val == rec_val2) 
          {
            join_records(rec_dest, dest, left_record, left_search, right_record, right_search);
            append_record(rec_dest, dest);
          }  
        }
      }
    }
  }
  release_record(rec_dest, dest);
  return dest->tbl;
}

/* copy fields to our new schema */
schema_p join_schema(schema_p const s,schema_p const r, char const* const dest_name) 
{
  if (!s) return 0;
  schema_p dest = new_schema(dest_name);
  for (field_desc_p fld = s->first; fld; fld = fld->next)
    add_field(dest, dup_field(fld));

  field_desc_p fld2 = r->first;
  for (; fld2; fld2 = fld2->next) 
  {
    if (get_field(dest, fld2->name) == NULL) 
    {
      add_field(dest, dup_field(fld2));
    }
  }
  return dest;
}

/* put fields in our new record without duplicates */
void join_records(record dest_r, schema_p dest_s, record src_r, schema_p src_s, record src_r2, schema_p src_s2) 
{
  field_desc_p src_f, dest_f, src_f2;
  size_t i = 0, j = 0;
  for (dest_f = dest_s->first; i < src_s->num_fields; dest_f = dest_f->next, i++) 
  {
    for (j = 0, src_f = src_s->first; strcmp(src_f->name, dest_f->name) != 0; j++, src_f = src_f->next);

    if (is_int_field(dest_f))
      assign_int_field(dest_r[i], *(int *)src_r[j]);
    else
      assign_str_field(dest_r[i], (char *)src_r[j]);
  }

  /* continue where left stopped */
  for (; dest_f; dest_f = dest_f->next, i++) 
  {
    for (j = 0, src_f2 = src_s2->first; strcmp(src_f2->name, dest_f->name) != 0; j++, src_f2 = src_f2->next);

    if (is_int_field(dest_f))
    {
      assign_int_field(dest_r[i], *(int *)src_r2[j]);
    } else 
    {
      assign_str_field(dest_r[i], (char *)src_r2[j]);
    }
  }
}



