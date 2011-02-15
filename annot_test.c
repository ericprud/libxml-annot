#include <stdio.h>
#include <string.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xmlschemas.h>
#include <libxml/schemasInternals.h>
#include <libxml/tree.h>

/*
cc test.c -Iinclude .libs/libxml2.so

LD_LIBRARY_PATH=./.libs/ ./a.out test/schemas/annot-register_0.xml

*/

extern void (*erics_annotation_schema_callback)(void *);


char *type_names[] =
{
    "XML_INVALID_ELEMENT_0",
    "XML_ELEMENT_NODE",
    "XML_ATTRIBUTE_NODE",
    "XML_TEXT_NODE",
    "XML_CDATA_SECTION_NODE",
    "XML_ENTITY_REF_NODE",
    "XML_ENTITY_NODE",
    "XML_PI_NODE",
    "XML_COMMENT_NODE",
    "XML_DOCUMENT_NODE",
    "XML_DOCUMENT_TYPE_NODE",
    "XML_DOCUMENT_FRAG_NODE",
    "XML_NOTATION_NODE",
    "XML_HTML_DOCUMENT_NODE",
    "XML_DTD_NODE",
    "XML_ELEMENT_DECL",
    "XML_ATTRIBUTE_DECL",
    "XML_ENTITY_DECL",
    "XML_NAMESPACE_DECL",
    "XML_XINCLUDE_START",
    "XML_XINCLUDE_END",
    "XML_DOCB_DOCUMENT_NODE",
};

/*
 * There are functions to get stuff out of node - such as xmlGetProp(node, xmlChar *name),
 * xmlNodeGetContent(), xmlGetNodePath(),
 */
void
walk_doc_tree(xmlNodePtr node, int level)
{
    int x;
    int is_annotation;
    xmlChar *c;

    while (node != NULL)
    {
        for (x = 0;x < level;x++)
            printf("  ");
        c = xmlNodeGetContent(node);
        printf("node %p type: %-20s node name: %-20s  content: \"%.40s\"\n",
               node, type_names[node->type], node->name, c);
        is_annotation = !strcmp(node->name, "annotation");
        if (is_annotation)
            printf("  Got an annotation!\n");

        walk_doc_tree(node->children, level + 1);
        node = node->next;
    }
}


/*
 * struct _xmlSchemaAnnot {
 *    struct _xmlSchemaAnnot *next;
 *    xmlNodePtr content;         /* the annotation * /
 * };
 * can be member of:
 *  xmlSchemaAttribute
 *  xmlSchemaWildcard
 *  xmlSchemaAttributeGroup
 *  xmlSchemaType
 *  xmlSchemaElement
 *  xmlSchemaFacet
 *  xmlSchemaNotation
 *  xmlSchema
 *
 * xmlSchemaTypePtr type;
 * xmlSchemaAddAnnotation((xmlSchemaAnnotItemPtr) type, ...
 * ((struct xmlSchemaWildcardPtr)xmlSchemaAnnotItemPtr)->annot
 *
 * Question:
 * Do all of the above xmlSchema* have ->annot in the same place?
 * No.
 *
 * Question:
 * xmlSchemaAttributePtr attr;
 * attr->annot;
 * Who is a consumer of the AttributePtr?
 * same for xmlSchemaAttributeUsePtr use->annot
 * and the above xmlSchema* structs.
 * and xmlSchemaPtr->annot
 *
 * xmlSchemaTypeDump prints out an xmlSchemaTypePtr.
 * Where do xmlSchemaTypePtrs come from?
 *
 */

void
walk_schema_tree(xmlSchemaPtr xmlSchema)
{
    xmlDocPtr doc;

    printf("name = \"%s\", target namespace = \"%s\", version = \"%s\"\n",
           xmlSchema->name, xmlSchema->targetNamespace, xmlSchema->version);
    printf("docptr = %p\n", xmlSchema->doc);
    printf("annot = %p\n", xmlSchema->annot);

    doc = xmlSchema->doc;

    printf("doc->type = %d\n", doc->type);
    printf("doc->children = %p\n", doc->children);
    printf("doc->next = %p\n", doc->next);
    if (doc->children)
        walk_doc_tree(doc->children, 0);
}

void annotation_callback(void *foo)
{
    printf("******************************** %s: %p\n", __FUNCTION__, foo);
}

int
main(int argc, char *argv[])
{
    xmlDocPtr docPtr = NULL;
    xmlNodePtr tree_trunk = NULL;
    char filename[100];
    xmlSchemaPtr wxschemas = NULL;

    if (argc != 2)
    {
        printf("gimme arg.\n");
        return -1;
    }        

    erics_annotation_schema_callback = annotation_callback;

    /* Read the schema. */
    {
        /* There is no visitibility into parserCtxt. */
	xmlSchemaParserCtxtPtr parserCtxt;

        /* parserCtxt->ctxtType is xmlSchemaTypePtr */

        snprintf(filename, 100, "%s.xsd", argv[1]);
        printf("\n----> Reading schema %s...\n", filename);

	parserCtxt = xmlSchemaNewParserCtxt(filename);
	xmlSchemaSetParserErrors(parserCtxt,
		(xmlSchemaValidityErrorFunc) fprintf,
		(xmlSchemaValidityWarningFunc) fprintf,
		stdout);
	wxschemas = xmlSchemaParse(parserCtxt);
	if (wxschemas == NULL)
        {
            printf("***** schema parsing failed!\n");
	}
	xmlSchemaFreeParserCtxt(parserCtxt);
        printf("<---- Schema read!\n\n");
    }

    /* Read the XML. */
    {
        snprintf(filename, 100, "%s.xml", argv[1]);
        printf("\n----> Reading XML %s...\n", filename);
        if ((docPtr = xmlReadFile(filename, NULL, 0)) == NULL)
        {
            printf("That didn't work out.\n");
            return -1;
        }
        printf("<---- XML read!\n\n");
    }

    {
        /* There is no visibility into schemaCtxt. */
	xmlSchemaValidCtxtPtr schemaCtxt;
	int ret;

        printf("\n----> Validating document %s...\n", filename);

        /* This sets up the schemaCtxt, including a pointer to wxschemas. */
	schemaCtxt = xmlSchemaNewValidCtxt(wxschemas);
	xmlSchemaSetValidErrors(schemaCtxt,
		(xmlSchemaValidityErrorFunc) fprintf,
		(xmlSchemaValidityWarningFunc) fprintf,
		stdout);
	ret = xmlSchemaValidateDoc(schemaCtxt, docPtr);	/* read me! */
	if (ret == 0)
        {
	    printf("%s validates\n", filename);
	}
        else if (ret > 0)
        {
	    printf("%s fails to validate\n", filename);
	}
        else
        {
	    printf("%s validation generated an internal error\n",
		   filename);
	}
	xmlSchemaFreeValidCtxt(schemaCtxt);
        printf("<---- Document validated!\n");

    }


#if 0
    /* why can't I just start with doc->children? */
    tree_trunk = xmlDocGetRootElement(docPtr);
#endif

    tree_trunk = docPtr->children;

    printf("\nWalking doc tree...\n");
    walk_doc_tree(tree_trunk, 0);
    printf("\n");

    printf("\nWalking schema tree...\n");
    walk_schema_tree(wxschemas);
    printf("\n");

    /*****************************************************************/
    /*****************************************************************/
    /*****************************************************************/
    /*****************************************************************/
    /* This will tell me, for example, how to decode sequences. */
    xmlSchemaDump(stdout, wxschemas);
    /*****************************************************************/
    /*****************************************************************/
    /*****************************************************************/
    /*****************************************************************/

    xmlFreeDoc(docPtr);

    xmlCleanupParser();

    return 0;
}


/*
 * _xmlAttribute
 * _xmlElement
 * _xmlDtd
 * _xmlAttr
 * _xmlNode
 * _xmlDoc
 * 
 * ... all have ->type as the second element, and appear to be somewhat interchangable.
 *
 *
 *
 */

#if 0

From tree.h:

struct _xmlDoc {
    void           *_private;	/* application data */
    xmlElementType  type;       /* XML_DOCUMENT_NODE, must be second ! */
    char           *name;	/* name/filename/URI of the document */
    struct _xmlNode *children;	/* the document tree */
    struct _xmlNode *last;	/* last child link */
    struct _xmlNode *parent;	/* child->parent link */
    struct _xmlNode *next;	/* next sibling link  */
    struct _xmlNode *prev;	/* previous sibling link  */
    struct _xmlDoc  *doc;	/* autoreference to itself */

    /* End of common part */
    int             compression;/* level of zlib compression */
    int             standalone; /* standalone document (no external refs) 
				     1 if standalone="yes"
				     0 if standalone="no"
				    -1 if there is no XML declaration
				    -2 if there is an XML declaration, but no
					standalone attribute was specified */
    struct _xmlDtd  *intSubset;	/* the document internal subset */
    struct _xmlDtd  *extSubset;	/* the document external subset */
    struct _xmlNs   *oldNs;	/* Global namespace, the old way */
    const xmlChar  *version;	/* the XML version string */
    const xmlChar  *encoding;   /* external initial encoding, if any */
    void           *ids;        /* Hash table for ID attributes if any */
    void           *refs;       /* Hash table for IDREFs attributes if any */
    const xmlChar  *URL;	/* The URI for that document */
    int             charset;    /* encoding of the in-memory content
				   actually an xmlCharEncoding */
    struct _xmlDict *dict;      /* dict used to allocate names or NULL */
    void           *psvi;	/* for type/PSVI informations */
    int             parseFlags;	/* set of xmlParserOption used to parse the
				   document */
    int             properties;	/* set of xmlDocProperties for this document
                                   set at the end of parsing */
};

/* This is one of the big ones */
struct _xmlNode {
    void           *_private;	/* application data */
    xmlElementType   type;	/* type number, must be second ! */
    const xmlChar   *name;      /* the name of the node, or the entity */
    struct _xmlNode *children;	/* parent->childs link */
    struct _xmlNode *last;	/* last child link */
    struct _xmlNode *parent;	/* child->parent link */
    struct _xmlNode *next;	/* next sibling link  */
    struct _xmlNode *prev;	/* previous sibling link  */
    struct _xmlDoc  *doc;	/* the containing document */

    /* End of common part */
    xmlNs           *ns;        /* pointer to the associated namespace */
    xmlChar         *content;   /* the content */
    struct _xmlAttr *properties;/* properties list */
    xmlNs           *nsDef;     /* namespace definitions on this node */
    void            *psvi;	/* for type/PSVI informations */
    unsigned short   line;	/* line number */
    unsigned short   extra;	/* extra data for XPath/XSLT */
};

struct _xmlAttr {
    void           *_private;	/* application data */
    xmlElementType   type;      /* XML_ATTRIBUTE_NODE, must be second ! */
    const xmlChar   *name;      /* the name of the property */
    struct _xmlNode *children;	/* the value of the property */
    struct _xmlNode *last;	/* NULL */
    struct _xmlNode *parent;	/* child->parent link */
    struct _xmlAttr *next;	/* next sibling link  */
    struct _xmlAttr *prev;	/* previous sibling link  */
    struct _xmlDoc  *doc;	/* the containing document */
    xmlNs           *ns;        /* pointer to the associated namespace */
    xmlAttributeType atype;     /* the attribute type if validating */
    void            *psvi;	/* for type/PSVI informations */
};

struct _xmlAttribute {
    void           *_private;	        /* application data */
    xmlElementType          type;       /* XML_ATTRIBUTE_DECL, must be second ! */
    const xmlChar          *name;	/* Attribute name */
    struct _xmlNode    *children;	/* NULL */
    struct _xmlNode        *last;	/* NULL */
    struct _xmlDtd       *parent;	/* -> DTD */
    struct _xmlNode        *next;	/* next sibling link  */
    struct _xmlNode        *prev;	/* previous sibling link  */
    struct _xmlDoc          *doc;       /* the containing document */

    struct _xmlAttribute  *nexth;	/* next in hash table */
    xmlAttributeType       atype;	/* The attribute type */
    xmlAttributeDefault      def;	/* the default */
    const xmlChar  *defaultValue;	/* or the default value */
    xmlEnumerationPtr       tree;       /* or the enumeration tree if any */
    const xmlChar        *prefix;	/* the namespace prefix if any */
    const xmlChar          *elem;	/* Element holding the attribute */
};

struct _xmlDtd {
    void           *_private;	/* application data */
    xmlElementType  type;       /* XML_DTD_NODE, must be second ! */
    const xmlChar *name;	/* Name of the DTD */
    struct _xmlNode *children;	/* the value of the property link */
    struct _xmlNode *last;	/* last child link */
    struct _xmlDoc  *parent;	/* child->parent link */
    struct _xmlNode *next;	/* next sibling link  */
    struct _xmlNode *prev;	/* previous sibling link  */
    struct _xmlDoc  *doc;	/* the containing document */

    /* End of common part */
    void          *notations;   /* Hash table for notations if any */
    void          *elements;    /* Hash table for elements if any */
    void          *attributes;  /* Hash table for attributes if any */
    void          *entities;    /* Hash table for entities if any */
    const xmlChar *ExternalID;	/* External identifier for PUBLIC DTD */
    const xmlChar *SystemID;	/* URI for a SYSTEM or PUBLIC DTD */
    void          *pentities;   /* Hash table for param entities if any */
};

From schema internals:

struct _xmlSchema {
    const xmlChar *name; /* schema name */
    const xmlChar *targetNamespace; /* the target namespace */
    const xmlChar *version;
    const xmlChar *id; /* Obsolete */
    xmlDocPtr doc;
    xmlSchemaAnnotPtr annot;
    int flags;

    xmlHashTablePtr typeDecl;
    xmlHashTablePtr attrDecl;
    xmlHashTablePtr attrgrpDecl;
    xmlHashTablePtr elemDecl;
    xmlHashTablePtr notaDecl;

    xmlHashTablePtr schemasImports;

    void *_private;        /* unused by the library for users or bindings */
    xmlHashTablePtr groupDecl;
    xmlDictPtr      dict;
    void *includes;     /* the includes, this is opaque for now */
    int preserve;        /* whether to free the document */
    int counter; /* used to give ononymous components unique names */
    xmlHashTablePtr idcDef; /* All identity-constraint defs. */
    void *volatiles; /* Obsolete */
};

element, text, and comment nodes?

#endif
