/* Configuration for an i386 running MS-DOS with djgpp/go32.  */

#include "dbxcoff.h"

/* Don't assume anything about the header files. */
#define NO_IMPLICIT_EXTERN_C

#define HANDLE_SYSV_PRAGMA

/* Enable parsing of #pragma pack(push,<n>) and #pragma pack(pop).  */
#define HANDLE_PRAGMA_PACK_PUSH_POP 1

#define YES_UNDERSCORES

#include "i386/gas.h"

/* Search for as.exe and ld.exe in DJGPP's binary directory. */ 
#define MD_EXEC_PREFIX "$DJDIR/bin/"

/* Correctly handle absolute filename detection in cp/xref.c */
#define FILE_NAME_ABSOLUTE_P(NAME) \
        (((NAME)[0] == '/') || ((NAME)[0] == '\\') || \
        (((NAME)[0] >= 'A') && ((NAME)[0] <= 'z') && ((NAME)[1] == ':')))

#ifdef CPP_PREDEFINES
#undef CPP_PREDEFINES
#endif
#define CPP_PREDEFINES "-Dunix -Di386 -DGO32 -DMSDOS \
  -Asystem(unix) -Asystem(msdos) -Acpu(i386) -Amachine(i386)"

/* We need to override link_command_spec in gcc.c so support -Tdjgpp.djl.
   This cannot be done in LINK_SPECS as that LINK_SPECS is processed
   before library search directories are known by the linker.
   This avoids problems when specs file is not available. An alternate way,
   suggested by Robert Hoehne, is to use SUBTARGET_EXTRA_SPECS instead.
*/ 

#undef LINK_COMMAND_SPEC
#define LINK_COMMAND_SPEC \
"%{!fsyntax-only: \
%{!c:%{!M:%{!MM:%{!E:%{!S:%(linker) %l %X %{o*} %{A} %{d} %{e*} %{m} %{N} %{n} \
\t%{r} %{s} %{t} %{u*} %{x} %{z} %{Z}\
\t%{!A:%{!nostdlib:%{!nostartfiles:%S}}}\
\t%{static:} %{L*} %D %o\
\t%{!nostdlib:%{!nodefaultlibs:%G %L %G}}\
\t%{!A:%{!nostdlib:%{!nostartfiles:%E}}}\
\t-Tdjgpp.djl %{T*}}}}}}}\n\
%{!c:%{!M:%{!MM:%{!E:%{!S:stubify %{v} %{o*:%*} %{!o*:a.out} }}}}}"
 
/* Make sure that gcc will not look for .h files in /usr/local/include 
   unless user explicitly requests it.  */
#undef LOCAL_INCLUDE_DIR

#undef EXTRA_SECTIONS
#define EXTRA_SECTIONS in_ctor, in_dtor

#undef EXTRA_SECTION_FUNCTIONS
#define EXTRA_SECTION_FUNCTIONS					\
  CTOR_SECTION_FUNCTION						\
  DTOR_SECTION_FUNCTION

#define CTOR_SECTION_FUNCTION					\
void								\
ctor_section ()							\
{								\
  if (in_section != in_ctor)					\
    {								\
      fprintf (asm_out_file, "\t.section .ctor\n");		\
      in_section = in_ctor;					\
    }								\
}

#define DTOR_SECTION_FUNCTION					\
void								\
dtor_section ()							\
{								\
  if (in_section != in_dtor)					\
    {								\
      fprintf (asm_out_file, "\t.section .dtor\n");		\
      in_section = in_dtor;					\
    }								\
}

#define ASM_OUTPUT_CONSTRUCTOR(FILE,NAME)	\
  do {						\
    ctor_section ();				\
    fprintf (FILE, "%s\t", ASM_LONG);		\
    assemble_name (FILE, NAME);			\
    fprintf (FILE, "\n");			\
  } while (0)

/* Allow (eg) __attribute__((section "locked")) to work */
#define ASM_OUTPUT_SECTION_NAME(FILE, DECL, NAME, RELOC)\
  do {						\
    fprintf (FILE, "\t.section %s\n", NAME);	\
  } while (0)

#define ASM_OUTPUT_DESTRUCTOR(FILE,NAME)	\
  do {						\
    dtor_section ();                   		\
    fprintf (FILE, "%s\t", ASM_LONG);		\
    assemble_name (FILE, NAME);              	\
    fprintf (FILE, "\n");			\
  } while (0)

/* Output at beginning of assembler file.  */
/* The .file command should always begin the output.  */

#undef ASM_FILE_START
#define ASM_FILE_START(FILE)						\
  do {									\
	output_file_directive (FILE, main_input_filename);		\
  } while (0)

/* This is how to output an assembler line
   that says to advance the location counter
   to a multiple of 2**LOG bytes.  */

#undef ASM_OUTPUT_ALIGN
#define ASM_OUTPUT_ALIGN(FILE,LOG) \
  if ((LOG) != 0) fprintf ((FILE), "\t.p2align %d\n", LOG)

/* djgpp has atexit ().  */
#undef HAVE_ATEXIT
#define HAVE_ATEXIT

/* djgpp automatically calls its own version of __main, so don't define one
   in libgcc, nor call one in main().  */
#define HAS_INIT_SECTION
