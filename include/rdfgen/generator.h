// Travis Whitaker 2013
// twhitak@its.jnj.com

#ifndef GENERATOR_H
#define GENERATOR_H

// Here we define the static parts of the RDF header:

// Define the XML version string:
#define XML_VERSION "<?xml version=\"1.0\"?>"

// Doctype string:
#define DOCTYPE "<!DOCTYPE rdf:RDF [<!ENTITY xsd \"http://www.w3.org/2001/XMLSchema#\">]>"

// RDF namespace string:
#define RDF_NAMESPACES "<rdf:RDF\n  xmlns:rdf=\"http://www.w3.org/1999/02/22-rdf-syntax-ns#\"\n  xmlns:rdfs=\"http://www.w3.org/2000/01/rdf-schema#\"\n  xml:base=\"http://rndusraabcdd2.rndus.na.jnj.com/ODS/ODS\"\n  xmlns:ODS=\"http://rndusraabcdd2.rndus.na.jnj.com/ODS/schema#\">"

// RDF NS prefix:
#define PREFIX "ODS"

// RDF base:
#define BASE "http://rndusraabcdd2.rndus.na.jnj.com/ODS/ODS"

#include <rdfgen/interface.h>

// Utility functions:
int checkLeaf(table_t *table);
int checkVirt(table_t *table);
int checkReq(table_t *table);

// No-assumption generator functions:
void genTriples(char *inputfile_map, unsigned long int *cursor, FILE *outputfile, field_t *row_buffer, table_t *table, unsigned long int *triples);
void genTriples_pifk(char *inputfile_map, unsigned long int *cursor, FILE *outputfile, field_t *row_buffer, table_t *table, unsigned long int *triples);
void genTriples_anon(char *inputfile_map, unsigned long int *cursor, FILE *outputfile, field_t *row_buffer, table_t *table, unsigned long int *triples);
void genTriples_anon_leaf(char *inputfile_map, unsigned long int *cursor, FILE *outputfile, field_t *row_buffer, table_t *table, unsigned long int *triples);
void genTriples_leaf(char *inputfile_map, unsigned long int *cursor, FILE *outputfile, field_t *row_buffer, table_t *table, unsigned long int *triples);

// Generator functions assuming no required columns:
void genTriples_no_req(char *inputfile_map, unsigned long int *cursor, FILE *outputfile, field_t *row_buffer, table_t *table, unsigned long int *triples);
void genTriples_pifk_no_req(char *inputfile_map, unsigned long int *cursor, FILE *outputfile, field_t *row_buffer, table_t *table, unsigned long int *triples);
void genTriples_anon_no_req(char *inputfile_map, unsigned long int *cursor, FILE *outputfile, field_t *row_buffer, table_t *table, unsigned long int *triples);
void genTriples_anon_leaf_no_req(char *inputfile_map, unsigned long int *cursor, FILE *outputfile, field_t *row_buffer, table_t *table, unsigned long int *triples);
void genTriples_leaf_no_req(char *inputfile_map, unsigned long int *cursor, FILE *outputfile, field_t *row_buffer, table_t *table, unsigned long int *triples);

// Generator functions assuming no virtual columns:
void genTriples_no_virt(char *inputfile_map, unsigned long int *cursor, FILE *outputfile, field_t *row_buffer, table_t *table, unsigned long int *triples);
void genTriples_pifk_no_virt(char *inputfile_map, unsigned long int *cursor, FILE *outputfile, field_t *row_buffer, table_t *table, unsigned long int *triples);
void genTriples_anon_no_virt(char *inputfile_map, unsigned long int *cursor, FILE *outputfile, field_t *row_buffer, table_t *table, unsigned long int *triples);
void genTriples_anon_leaf_no_virt(char *inputfile_map, unsigned long int *cursor, FILE *outputfile, field_t *row_buffer, table_t *table, unsigned long int *triples);
void genTriples_leaf_no_virt(char *inputfile_map, unsigned long int *cursor, FILE *outputfile, field_t *row_buffer, table_t *table, unsigned long int *triples);

// Generator functions assuming no virtual columns and no required columns:
void genTriples_no_virt_no_req(char *inputfile_map, unsigned long int *cursor, FILE *outputfile, field_t *row_buffer, table_t *table, unsigned long int *triples);
void genTriples_pifk_no_virt_no_req(char *inputfile_map, unsigned long int *cursor, FILE *outputfile, field_t *row_buffer, table_t *table, unsigned long int *triples);
void genTriples_anon_no_virt_no_req(char *inputfile_map, unsigned long int *cursor, FILE *outputfile, field_t *row_buffer, table_t *table, unsigned long int *triples);
void genTriples_anon_leaf_no_virt_no_req(char *inputfile_map, unsigned long int *cursor, FILE *outputfile, field_t *row_buffer, table_t *table, unsigned long int *triples);
void genTriples_leaf_no_virt_no_req(char *inputfile_map, unsigned long int *cursor, FILE *outputfile, field_t *row_buffer, table_t *table, unsigned long int *triples);

#endif
