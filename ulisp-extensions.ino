/*
 User Extensions
*/
  
// Definitions
object *fn_now (object *args, object *env) {
  (void) env;
  static unsigned long Offset;
  unsigned long now = millis()/1000;
  int nargs = listlength(args);

  // Set time
  if (nargs == 3) {
    Offset = (unsigned long)((checkinteger(first(args))*60 + checkinteger(second(args)))*60
      + checkinteger(third(args)) - now);
  } else if (nargs > 0) error2(PSTR("wrong number of arguments"));
  
  // Return time
  unsigned long secs = Offset + now;
  object *seconds = number(secs%60);
  object *minutes = number((secs/60)%60);
  object *hours = number((secs/3600)%24);
  return cons(hours, cons(minutes, cons(seconds, NULL)));
}

object *fn_sym_def (object *args, object *env) {
  (void) env;
  object *obj = first(args);
  pfun_t pfun = pstreamfun(cdr(args));
  #if defined(gfxsupport)
  if (pfun == gfxwrite) ppwidth = GFXPPWIDTH;
  #endif
  object *pair = findvalue(obj, env);
  object *var = car(pair);
  object *val = cdr(pair);
  pln(pfun);
  if (consp(val) && symbolp(car(val)) && builtin(car(val)->name) == LAMBDA) {
    superprint(cons(bsymbol(DEFUN), cons(var, cdr(val))), 0, false, pfun);
  } else {
    superprint(cons(bsymbol(DEFVAR), cons(var, cons(quote(val), NULL))), 0, false, pfun);
  }
  pln(pfun);
  ppwidth = PPWIDTH;
  return bsymbol(NOTHING);
}

object *fn_rename_file(object *args, object *env) {
  (void) env;
  #if defined(sdcardsupport)
  sd_begin();
  object *path1 = first(args);
  object *path2 = second(args);
  char buffer1[BUFFERSIZE];
  char buffer2[BUFFERSIZE];
  if (SD.rename(MakeFilename(path1, buffer1), MakeFilename(path2, buffer2))) {
    return path2;
  }
  else { 
    error2(PSTR("Unable to rename file."));
    return nil;
  }
  #endif
}

object *fn_delete_file(object *args, object *env) {
  (void) env;
  #if defined(sdcardsupport)
  sd_begin();
  object *filename = first(args);
  char buffer[BUFFERSIZE];
  if (SD.remove(MakeFilename(filename, buffer))) {
    return filename;
  }
  else { 
    error2(PSTR("Unable to delete file."));
    return nil;
  }
  #endif
}

object *fn_make_dir(object *args, object *env) {
  (void) env;
  #if defined(sdcardsupport)
  sd_begin();
  object *path = first(args);
  char buffer[BUFFERSIZE];
  if (SD.mkdir(MakeFilename(path, buffer))) {
    return path;
  }
  else { 
    error2(PSTR("Unable to make directory."));
    return nil;
  }
  #endif
}

object *fn_remove_dir(object *args, object *env) {
  (void) env;
  #if defined(sdcardsupport)
  sd_begin();
  object *path = first(args);
  char buffer[BUFFERSIZE];
  if (SD.rmdir(MakeFilename(path, buffer))) {
    return path;
  }
  else { 
    error2(PSTR("Unable to remove directory."));
    return nil;
  }
  #endif
}

object *fn_list_dir(object *arg, object *env) {
  (void) env;
  #if defined(sdcardsupport)
  sd_begin();
  File root;
  if (stringp(arg)) {
    char buffer[BUFFERSIZE];
    root = SD.open(MakeFilename(arg, buffer));
  } else {
    root = SD.open("/");
  }
  if(!root){
    error2(PSTR("Failed to open directory"));
    return nil;
  }
  if(!root.isDirectory()) {
    error2(PSTR("Not a directory"));
    return nil;
  }
  object *result = cons(NULL, NULL);
  object *ptr = result;
  while (true) {
    File entry = root.openNextFile();
    if (!entry) break;
    object *filename = lispstring((char*)entry.name());
    cdr(ptr) = cons(filename, NULL);
    ptr = cdr(ptr);
  };
  root.close();
  return cdr(result);
  #endif
}

object *fn_directoryp(object *args, object *env) {
  (void) env;
  #if defined(sdcardsupport)
  sd_begin();
  object *dirname = first(args);
  char buffer[BUFFERSIZE];
  File file = SD.open(MakeFilename(dirname, buffer));
  if(!file){
    error2(PSTR("Failed to find directory"));
    return nil;
  }
  if(!file.isDirectory()) {
    return nil;
  } else {
    return lispstring((char*)file.name());
  };
  #endif
}


// Symbol names
const char stringnow[] PROGMEM = "now";
const char string_sym_def[] PROGMEM = "symbol-def";
const char string_delete_file[] PROGMEM = "delete-file";
const char string_rename_file[] PROGMEM = "rename-file";
const char string_make_dir[] PROGMEM = "make-directory";
const char string_remove_dir[] PROGMEM = "remove-directory";
const char string_list_dir[] PROGMEM = "list-dir";
const char string_directoryp[] PROGMEM = "directoryp";

// Documentation strings
const char docnow[] PROGMEM = "(now [hh mm ss])\n"
"Sets the current time, or with no arguments returns the current time\n"
"as a list of three integers (hh mm ss).";

const char doc_sym_def[] PROGMEM = "(symbol-def symbol [str])\n"
"Prints the definition of variable or function defined in ulisp using the pretty printer."
"If str is specified it prints to the specified stream. It returns no value.";

const char doc_delete_file[] PROGMEM = "removes a file from the sd-card given it's filepath";

const char doc_rename_file[] PROGMEM = "renames file from old filepath to new, can be used to move a file";

const char doc_make_dir[] PROGMEM = "make directory path";

const char doc_remove_dir[] PROGMEM = "removes a directory given in path";

const char doc_list_dir[] PROGMEM = "list the contents of a given directory";

const char doc_directoryp[] PROGMEM = "test whether a file is a directory";

// Symbol lookup table
const tbl_entry_t lookup_table2[] PROGMEM = {
  { stringnow, fn_now, 0203, docnow },
  { string_sym_def, fn_sym_def, 0212, doc_sym_def },
  { string_delete_file, fn_delete_file, 0211, doc_delete_file },
  { string_rename_file, fn_rename_file, 0222, doc_rename_file },
  { string_make_dir, fn_make_dir, 0211, doc_make_dir },
  { string_remove_dir, fn_remove_dir, 0211, doc_remove_dir },
  { string_list_dir, fn_list_dir, 0201, doc_list_dir },
  { string_directoryp, fn_directoryp, 0211, doc_directoryp }
};

// Table cross-reference functions

tbl_entry_t *tables[] = {lookup_table, lookup_table2};
const unsigned int tablesizes[] = { arraysize(lookup_table), arraysize(lookup_table2) };

const tbl_entry_t *table (int n) {
  return tables[n];
}

unsigned int tablesize (int n) {
  return tablesizes[n];
}
