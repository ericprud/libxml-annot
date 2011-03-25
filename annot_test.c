#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xmlschemas.h>
#include <libxml/schemasInternals.h>
#include <libxml/tree.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>

/*
  TODO:
  - require -DREGISTER_ANNOTATIONS to enable this API
*/

  /**
   * annotationParseEvent - app callback when XML Schema parser encounters an
   * annotation attribute or appinfo element. The app may accept or discard the
   * annotation. Discarding will disable subsequent callbacks for that element.
   * 
   * @loc: a location context which can be passed to the app again when
   * validating such an element in an XML document.
   * @ns: the namespace of the annotated attribute.
   * @lname: the localname of the annotated attribute.
   * @node: the DOM node of the annotation. For an annotation attribute,
   *        this will be the DOM attribute node; for an appinfo element,
   *        it will be that element in the appinfo.
   *
   * @returns: true: accepted, false: discard.
  typedef bool (annotationParseEvent)(LOCATION loc, namespace ns,
                                      localname lname, NODE node);
   */

  /**
   * validateAnnotatedElement - app callback after XML Schema validator
   * validates an annotated element.
   * 
   * @loc: a location context which was passed to the app during schema parsing
   *       via the annotationParseEvent callback.
   * @node: the DOM node of the element being validated.
  typedef void (validateAnnotatedElement)(LOCATION loc, NODE node);
   */

  /**
   * generateAnnotatedElement - app callback after XML Schema validator
   * generates an annotated element.
   * 
   * @loc: a location context which was passed to the app during schema parsing
   *       via the annotationParseEvent callback.
   * @returns: the new DOM node for the generated element.
  typedef Node (generateAnnotatedElement)(LOCATION loc);
   */

  /**
  ErrCode register_annotationCallbacks(annotationParseEvent* p,
                                       validateAnnotatedElement* v,
                                       generatedAnnotatedElement* g);
  */

void walk_doc_tree(xmlNodePtr, int);
void walk_schema_tree(xmlSchemaPtr);

/*
 * It looks like the validation is done differently for SAX vs. DOM.
 *
 * See xmlSchemaVStart in xmlschemas.c
 */

const char *type_names[] =
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
    const xmlChar* empty = (xmlChar*)"";
    xmlChar* prefix = empty;
    if (node->ns) {
	prefix = xmlMalloc((strlen((const char*)node->ns->href) * sizeof(xmlChar)) + 3);
	sprintf((char*)prefix, "{%s}", node->ns->href);
    }

    if (node->type == XML_ELEMENT_NODE) {
	xmlNodePtr att = node->properties;	/* warning: initialization from incompatible pointer type ?? */
	xmlChar* closeMe = node->name;		/* warning: assignment discards qualifiers from pointer target type ?? */
	printf("<%s%s", prefix, closeMe);
	while (att != NULL) {
	    walk_doc_tree(att, level + 1);
	    att = att->next;
	}
	printf(">");

	node = node->children;
	while (node != NULL) {
	    walk_doc_tree(node, level + 1);
	    node = node->next;
	}

	if (closeMe != NULL)
	    printf("</%s%s>", prefix, closeMe);
    } else if (node->type == XML_ATTRIBUTE_NODE) {
	printf(" %s%s=\"%s\"", prefix, node->name, xmlNodeGetContent(node));
    } else if (node->type == XML_TEXT_NODE) {
	printf("%s", xmlNodeGetContent(node));
    } else
	printf("node %p type: %-20s node name: %s%-20s\n",
	       (void *)node, type_names[node->type], prefix, node->name);

    if (prefix != empty)
	xmlFree(prefix);
}


/*
 * struct _xmlSchemaAnnot {
 *    struct _xmlSchemaAnnot *next;
 *    xmlNodePtr content;         / * the annotation * /
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
walk_schema_tree(xmlSchemaPtr xmlschema)
{
    xmlDocPtr doc;

    printf("name = \"%s\", target namespace = \"%s\", version = \"%s\"\n",
           xmlschema->name, xmlschema->targetNamespace, xmlschema->version);
    printf("docptr = %p\n", (void *)xmlschema->doc);
    printf("annot = %p\n", (void *)xmlschema->annot);

    doc = xmlschema->doc;

    printf("doc->type = %d\n", doc->type);
    printf("doc->children = %p\n", (void *)doc->children);
    printf("doc->next = %p\n", (void *)doc->next);
    if (doc->children)
        walk_doc_tree(doc->children, 0);
}

/*
 * TODO 2-20:
 *
 * I should also get a callback for my:myLocalName, since it's a
 * foreign namespace.
 *
 * walk through each attribute, ignore prefix, any that aren't in a
 * known namespace get a callback too
 *
 * So far XMLSchema is the only known one.
 *
 * xmlAttr->ns has attribute namespace.
 * xmlNode->ns has node namespace.
 *
 * Go through attributes at the end of xmlSchemaParseElement and do
 * callbacks then.
 */


/*
 * These are the two callbacks used by xmlschema.c.  These should be
 * set via an API.
 */
extern int (*xmlSchemaAnnotationSchemaCallback)(void *, xmlNodePtr);
extern int (*xmlSchemaAnnotationInstanceCallback)(void *, xmlNodePtr);

int schema_annotation_callback(void *, xmlNodePtr);
int instance_annotation_callback(void *, xmlNodePtr);

xmlHashTablePtr Handle2Path = 0;

/*
 * This is called by annotation_callback() in xmlschemas.c when an
 * annotation is encountered while reading the schema.
 */
int schema_annotation_callback(void *handle, xmlNodePtr node)
{
    xmlChar* content = xmlNodeGetContent(node);
    char* key;
    if (node->ns == NULL
	|| strcmp("myNamespace", (const char*)node->ns->href)
	|| strcmp("path", (const char*)node->name))
	return 0;

    printf("\noooooooooooo %s: %p \"%s\"\n", __FUNCTION__, handle, content);
    walk_doc_tree(node, 0);
    printf("\n");

    key = xmlMalloc(20);
    sprintf(key, "%p", handle);
    xmlHashAddEntry(Handle2Path, (xmlChar*)key, content);

    return 1;
}

/*
 * This is called by ??? when reading an annotation is encountered
 * while reading the instance data.
 */
int instance_annotation_callback(void *handle, xmlNodePtr node)
{
    int i;
    char key[20];
    xmlChar* path;
    xmlDocPtr doc = node->doc;
    xmlXPathContextPtr xpathCtx = xmlXPathNewContext(doc);
    xmlXPathObjectPtr xpathObj;

    sprintf(key, "%p", handle);
    path = (xmlChar*)xmlHashLookup(Handle2Path, (xmlChar*)key);
    assert(path != NULL);

    printf("\noooooooooooo %s: %p %p\n", __FUNCTION__, handle, (void*)node);
    walk_doc_tree(node, 0);
    printf("\n");
    xpathCtx->node = node;
    xpathObj = xmlXPathEvalExpression(path, xpathCtx);
    if (xpathObj->nodesetval != NULL)
	for (i = 0; i < xpathObj->nodesetval->nodeNr; ++i)
	    printf("xpath(\"%s\") => %s\n", path,
		   xmlNodeGetContent(xpathObj->nodesetval->nodeTab[i]));
    return 1;
}

int
test(const char*);

int
main(int argc, char *argv[])
{
    int arg, ret;
    if (argc < 2)
    {
        printf("Argument needed - the name, minus extension, of a schema and xml doc.\n");
        return -1;
    }        

    xmlSchemaAnnotationSchemaCallback = schema_annotation_callback;
    xmlSchemaAnnotationInstanceCallback = instance_annotation_callback;
    for (arg = 1; arg < argc; ++arg)
	if ((ret = test(argv[arg])) != 0)
	    return ret;
    return 0;
}

int
test (const char* base) {
    xmlDocPtr docPtr = NULL;
    char filename[100];
    xmlSchemaPtr wxschemas = NULL;

    Handle2Path = xmlHashCreate(0);

    /* Read the schema. */
    {
        /* There is no visitibility into parserCtxt. */
	xmlSchemaParserCtxtPtr parserCtxt;

        /* parserCtxt->ctxtType is xmlSchemaTypePtr */

        snprintf(filename, 100, "%s.xsd", base);
        printf("\n\n\n----------------------------------------------------------------\n\n\n");
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
        printf("\n<---- Schema read!\n\n");
    }

    /* Read the XML. */
    {
        snprintf(filename, 100, "%s.xml", base);
        printf("----------------------------------------------------------------\n");
        printf("\n----> Reading XML %s...\n", filename);
        if ((docPtr = xmlReadFile(filename, NULL, 0)) == NULL)
        {
            printf("That didn't work out.\n");
            return -1;
        }
        printf("<---- XML read!\n");
    }

    {
        /* There is no visibility into schemaCtxt. */
	xmlSchemaValidCtxtPtr schemaCtxt;
	int ret;

        printf("\n----------------------------------------------------------------\n");
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
        printf("\n<---- Document validated!\n");

    }


#if 0
    /* why can't I just start with doc->children? */
    tree_trunk = xmlDocGetRootElement(docPtr);
#endif

#if 0
    tree_trunk = docPtr->children;

    printf("\n\n\n----------------------------------------------------------------\n\n\n");
    printf("\nWalking doc tree...\n");
    walk_doc_tree(tree_trunk, 0);
    printf("\n");
#endif
    printf("\n\n\n----------------------------------------------------------------\n\n\n");
    printf("\nWalking schema tree...\n");
    walk_schema_tree(wxschemas);
    printf("\n");

    /*****************************************************************/
    /*****************************************************************/
    /*****************************************************************/
    /*****************************************************************/
    /* This will tell me, for example, how to decode sequences. */
    printf("\n\n\n----------------------------------------------------------------\n\n\n");
    xmlSchemaDump(stdout, wxschemas);
    /*****************************************************************/
    /*****************************************************************/
    /*****************************************************************/
    /*****************************************************************/

    xmlFreeDoc(docPtr);

    xmlCleanupParser();

    xmlHashFree(Handle2Path, NULL);

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
