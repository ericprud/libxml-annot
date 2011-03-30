/*
 * Summary: incomplete XML Schemas structure implementation
 * Description: interface to the XML Schemas handling and schema validity
 *              checking, it is incomplete right now.
 *
 * Copy: See Copyright for the status of this software.
 *
 * Author: Daniel Veillard
 */


#ifndef __XML_SCHEMA_H__
#define __XML_SCHEMA_H__

#include <libxml/xmlversion.h>

#ifdef LIBXML_SCHEMAS_ENABLED

#include <libxml/tree.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * This error codes are obsolete; not used any more.
 */
typedef enum {
    XML_SCHEMAS_ERR_OK		= 0,
    XML_SCHEMAS_ERR_NOROOT	= 1,
    XML_SCHEMAS_ERR_UNDECLAREDELEM,
    XML_SCHEMAS_ERR_NOTTOPLEVEL,
    XML_SCHEMAS_ERR_MISSING,
    XML_SCHEMAS_ERR_WRONGELEM,
    XML_SCHEMAS_ERR_NOTYPE,
    XML_SCHEMAS_ERR_NOROLLBACK,
    XML_SCHEMAS_ERR_ISABSTRACT,
    XML_SCHEMAS_ERR_NOTEMPTY,
    XML_SCHEMAS_ERR_ELEMCONT,
    XML_SCHEMAS_ERR_HAVEDEFAULT,
    XML_SCHEMAS_ERR_NOTNILLABLE,
    XML_SCHEMAS_ERR_EXTRACONTENT,
    XML_SCHEMAS_ERR_INVALIDATTR,
    XML_SCHEMAS_ERR_INVALIDELEM,
    XML_SCHEMAS_ERR_NOTDETERMINIST,
    XML_SCHEMAS_ERR_CONSTRUCT,
    XML_SCHEMAS_ERR_INTERNAL,
    XML_SCHEMAS_ERR_NOTSIMPLE,
    XML_SCHEMAS_ERR_ATTRUNKNOWN,
    XML_SCHEMAS_ERR_ATTRINVALID,
    XML_SCHEMAS_ERR_VALUE,
    XML_SCHEMAS_ERR_FACET,
    XML_SCHEMAS_ERR_,
    XML_SCHEMAS_ERR_XXX
} xmlSchemaValidError;

/*
* ATTENTION: Change xmlSchemaSetValidOptions's check
* for invalid values, if adding to the validation
* options below.
*/
/**
 * xmlSchemaValidOption:
 *
 * This is the set of XML Schema validation options.
 */
typedef enum {
    XML_SCHEMA_VAL_VC_I_CREATE			= 1<<0
	/* Default/fixed: create an attribute node
	* or an element's text node on the instance.
	*/
} xmlSchemaValidOption;

/*
    XML_SCHEMA_VAL_XSI_ASSEMBLE			= 1<<1,
	* assemble schemata using
	* xsi:schemaLocation and
	* xsi:noNamespaceSchemaLocation
*/

/**
 * The schemas related types are kept internal
 */
typedef struct _xmlSchema xmlSchema;
typedef xmlSchema *xmlSchemaPtr;

/**
 * xmlSchemaValidityErrorFunc:
 * @ctx: the validation context
 * @msg: the message
 * @...: extra arguments
 *
 * Signature of an error callback from an XSD validation
 */
typedef void (XMLCDECL *xmlSchemaValidityErrorFunc) (void *ctx, const char *msg, ...) LIBXML_ATTR_FORMAT(2,3);

/**
 * xmlSchemaValidityWarningFunc:
 * @ctx: the validation context
 * @msg: the message
 * @...: extra arguments
 *
 * Signature of a warning callback from an XSD validation
 */
typedef void (XMLCDECL *xmlSchemaValidityWarningFunc) (void *ctx, const char *msg, ...) LIBXML_ATTR_FORMAT(2,3);

/**
 * xmlNotifyValidatedElement - app callback after XML Schema validator
 * validates a given element.
 * 
 * @handle: a location context which was passed to the app during schema parsing
 *          via the xmlAnnotationParseEvent callback.
 * @node: the DOM node of the element being validated.
 *
 * @returns: XML_ERR_OK if everything is OK, else some xmlParserErrors (see
 *           include/libxml/xmlerror.h)
 */
typedef xmlParserErrors
(XMLCDECL xmlNotifyValidatedElement)(void* handle, xmlNodePtr node);

/**
 * annotationParseEvent - app callback when XML Schema parser encounters an
 * annotation attribute or appinfo element. The app may accept or discard the
 * annotation. Discarding will disable subsequent callbacks for that element.
 * 
 * @handle: a context which will be passed to xmlNotifyValidatedElement
 *          when validating such an element in an XML document.
 * @annotation: the DOM node of the annotation. For an annotation attribute,
 *              this will be the DOM attribute node; for an appinfo element,
 *              it will be that element in the appinfo.
 *
 * @returns: a function pointer of type validateAnnotatedElement, or NULL if
 *           no callback is expected.
 */
typedef xmlNotifyValidatedElement*
(XMLCDECL xmlAnnotationParseEvent)(void * handle, xmlNodePtr annotation);

/**
 * generateAnnotatedElement - app callback after XML Schema validator
 * generates an annotated element.
 * 
 * @handle: a context which will be passed to xmlNotifyValidatedElement
 *          when validating such an element in an XML document.
 * @parent: the parent element if this prospective element. (The callback
 *          need not add the returned element to the parent's children.)
 * @ctx: the generation context
 * @returns: the new DOM node for the generated element.
*/
typedef xmlNodePtr
(XMLCDECL xmlGenerateElement)(void* handle, xmlNodePtr parent, void* ctx);

/**
 * A schemas validation context
 */
typedef struct _xmlSchemaParserCtxt xmlSchemaParserCtxt;
typedef xmlSchemaParserCtxt *xmlSchemaParserCtxtPtr;

typedef struct _xmlSchemaValidCtxt xmlSchemaValidCtxt;
typedef xmlSchemaValidCtxt *xmlSchemaValidCtxtPtr;

/*
 * Interfaces for parsing.
 */
XMLPUBFUN xmlSchemaParserCtxtPtr XMLCALL
	    xmlSchemaNewParserCtxt	(const char *URL);
XMLPUBFUN xmlSchemaParserCtxtPtr XMLCALL
	    xmlSchemaNewMemParserCtxt	(const char *buffer,
					 int size);
XMLPUBFUN xmlSchemaParserCtxtPtr XMLCALL
	    xmlSchemaNewDocParserCtxt	(xmlDocPtr doc);
XMLPUBFUN void XMLCALL
	    xmlSchemaFreeParserCtxt	(xmlSchemaParserCtxtPtr ctxt);
XMLPUBFUN void XMLCALL
	    xmlSchemaSetParserErrors	(xmlSchemaParserCtxtPtr ctxt,
					 xmlSchemaValidityErrorFunc err,
					 xmlSchemaValidityWarningFunc warn,
					 void *ctx);
XMLPUBFUN void XMLCALL
	    xmlSchemaSetParserStructuredErrors(xmlSchemaParserCtxtPtr ctxt,
					 xmlStructuredErrorFunc serror,
					 void *ctx);
XMLPUBFUN int XMLCALL
		xmlSchemaGetParserErrors(xmlSchemaParserCtxtPtr ctxt,
					xmlSchemaValidityErrorFunc * err,
					xmlSchemaValidityWarningFunc * warn,
					void **ctx);
XMLPUBFUN int XMLCALL
		xmlSchemaIsValid	(xmlSchemaValidCtxtPtr ctxt);

XMLPUBFUN xmlSchemaPtr XMLCALL
	    xmlSchemaParse		(xmlSchemaParserCtxtPtr ctxt);
XMLPUBFUN void XMLCALL
	    xmlSchemaFree		(xmlSchemaPtr schema);
#ifdef LIBXML_OUTPUT_ENABLED
XMLPUBFUN void XMLCALL
	    xmlSchemaDump		(FILE *output,
					 xmlSchemaPtr schema);
#endif /* LIBXML_OUTPUT_ENABLED */
/*
 * Interfaces for validating
 */
XMLPUBFUN void XMLCALL
	    xmlSchemaSetValidErrors	(xmlSchemaValidCtxtPtr ctxt,
					 xmlSchemaValidityErrorFunc err,
					 xmlSchemaValidityWarningFunc warn,
					 void *ctx);
XMLPUBFUN void XMLCALL
	    xmlSchemaSetValidStructuredErrors(xmlSchemaValidCtxtPtr ctxt,
					 xmlStructuredErrorFunc serror,
					 void *ctx);
XMLPUBFUN int XMLCALL
	    xmlSchemaGetValidErrors	(xmlSchemaValidCtxtPtr ctxt,
					 xmlSchemaValidityErrorFunc *err,
					 xmlSchemaValidityWarningFunc *warn,
					 void **ctx);
XMLPUBFUN int XMLCALL
	    xmlSchemaSetValidOptions	(xmlSchemaValidCtxtPtr ctxt,
					 int options);
XMLPUBFUN int XMLCALL
	    xmlSchemaValidCtxtGetOptions(xmlSchemaValidCtxtPtr ctxt);

XMLPUBFUN xmlSchemaValidCtxtPtr XMLCALL
	    xmlSchemaNewValidCtxt	(xmlSchemaPtr schema);
XMLPUBFUN void XMLCALL
	    xmlSchemaFreeValidCtxt	(xmlSchemaValidCtxtPtr ctxt);
XMLPUBFUN int XMLCALL
	    xmlSchemaValidateDoc	(xmlSchemaValidCtxtPtr ctxt,
					 xmlDocPtr instance);
XMLPUBFUN int XMLCALL
            xmlSchemaValidateOneElement (xmlSchemaValidCtxtPtr ctxt,
			                 xmlNodePtr elem);
XMLPUBFUN int XMLCALL
	    xmlSchemaValidateStream	(xmlSchemaValidCtxtPtr ctxt,
					 xmlParserInputBufferPtr input,
					 xmlCharEncoding enc,
					 xmlSAXHandlerPtr sax,
					 void *user_data);
XMLPUBFUN int XMLCALL
	    xmlSchemaValidateFile	(xmlSchemaValidCtxtPtr ctxt,
					 const char * filename,
					 int options);

XMLPUBFUN xmlParserCtxtPtr XMLCALL
	    xmlSchemaValidCtxtGetParserCtxt(xmlSchemaValidCtxtPtr ctxt);

/*
 * Interface to insert Schemas SAX validation in a SAX stream
 */
typedef struct _xmlSchemaSAXPlug xmlSchemaSAXPlugStruct;
typedef xmlSchemaSAXPlugStruct *xmlSchemaSAXPlugPtr;

XMLPUBFUN xmlSchemaSAXPlugPtr XMLCALL
            xmlSchemaSAXPlug		(xmlSchemaValidCtxtPtr ctxt,
					 xmlSAXHandlerPtr *sax,
					 void **user_data);
XMLPUBFUN int XMLCALL
            xmlSchemaSAXUnplug		(xmlSchemaSAXPlugPtr plug);

/*
 * Interface for tree manipulation
 */
XMLPUBFUN void XMLCALL
            xmlSchemaSetParserAnnotation(xmlSchemaParserCtxtPtr ctxt,
					 xmlAnnotationParseEvent* annot,
					 void *ctx);

XMLPUBFUN void XMLCALL
            xmlSchemaSetValidNotification
                                        (xmlSchemaValidCtxtPtr ctxt,
					 xmlNotifyValidatedElement gen,
					 void *ctx);
XMLPUBFUN void XMLCALL
            xmlSchemaSetGeneratorCallback
                                        (xmlSchemaValidCtxtPtr ctxt,
					 xmlGenerateElement gen,
					 void *ctx);

#ifdef __cplusplus
}
#endif

#endif /* LIBXML_SCHEMAS_ENABLED */
#endif /* __XML_SCHEMA_H__ */
