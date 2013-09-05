// Travis Whitaker 2013
// twhitak@its.jnj.com

#ifndef GENERATOR_H
#define GENERATOR_H

// Here we define the static parts of the RDF header:

// Define the XML version string:
#define XML_VERSION "<?xml version=\"1.0\"?>"

// Doctype string:
#define DOCTYPE "<!DOCTYPE rdf:RDF [<!ENTITY xsd \"http://www.w3.org/2001/XMLSchema#\">]>"

//RDF namespace string:
#define RDF_NAMESPACES "<rdf:RDF\n  xmlns:rdf=\"http://www.w3.org/1999/02/22-rdf-syntax-ns#\"\n  xmlns:rdfs=\"http://www.w3.org/2000/01/rdf-schema#\"\n  xml:base=\"http://www.jnj.com/DWH/schema\"\n  xmlns:DWH=\"http://example.org/schemas/vehicles#\">"



#endif
