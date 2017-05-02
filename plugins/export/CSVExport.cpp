/**
 *
 * This file is part of Tulip (www.tulip-software.org)
 *
 * Authors: David Auber and the Tulip development Team
 * from LaBRI, University of Bordeaux 1 and Inria Bordeaux - Sud Ouest
 *
 * Tulip is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation, either version 3
 * of the License, or (at your option) any later version.
 *
 * Tulip is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 */
#include "CSVExport.h"

#include <tulip/StringCollection.h>
#include <tulip/Graph.h>
#include <tulip/StringProperty.h>
#include <tulip/BooleanProperty.h>

PLUGIN(CsvExport)

using namespace tlp;
using namespace std;

static const char * paramHelp[] = {
  // the type of element to export
  "This parameter enables to choose the type of graph elements to export",
  // export selection
  "This parameter indicates if only selected elements have to be exported",
  // export id of graph elements
  "This parameter indicates if the id of graph elements has to be exported",
  // the field separator
  "This parameter enables to choose the field separator",
  // the field separator custom value
  "This parameter allows to use a custom field separator. The field separator parameter must set to 'Custom'",
  // the string delimiter
  "This parameter enables to choose the string delimiter",
  // export visual properties selection
  "This parameter indicates if the visual properties of Tulip will be exported",
  // export selection property
  "This parameters enables to choose the property used for the selection"
};

#define ELT_TYPE "Type of elements"
#define ELT_TYPES "nodes;edges;both"
#define NODE_TYPE 0
#define EDGE_TYPE 1
#define BOTH_TYPES 2

#define EXPORT_SELECTION "Export selection"

#define EXPORT_ID "Export id"

#define EXPORT_VISUAL_PROPERTIES "Export visual properties"

#define FIELD_SEPARATOR "Field separator"
#define FIELD_SEPARATORS " \\; ; , ;Tab;Space;Custom"
#define CUSTOM_SEPARATOR 4
#define COMMA_SEPARATOR 1
#define TAB_SEPARATOR 2
#define SPACE_SEPARATOR 3
#define SEMICOLON_SEPARATOR 0
#define FIELD_SEPARATOR_CUSTOM "Custom separator"
#define CUSTOM_MARK ";"

#define STRING_DELIMITER "String delimiter"
#define STRING_DELIMITERS " \" ; ' "
#define DBL_QUOTE_DELIMITER 0
#define QUOTE_DELIMITER 1

//================================================================================
CsvExport::CsvExport(const PluginContext *context):ExportModule(context) {
  addInParameter<StringCollection>(ELT_TYPE, paramHelp[0], ELT_TYPES);
  addInParameter<bool>(EXPORT_SELECTION, paramHelp[1], "false");
  addInParameter<BooleanProperty>("Export selection property", paramHelp[7], "viewSelection");
  addInParameter<bool>(EXPORT_ID, paramHelp[2], "false");
  addInParameter<bool>(EXPORT_VISUAL_PROPERTIES, paramHelp[6], "false");
  addInParameter<StringCollection>(FIELD_SEPARATOR, paramHelp[3], FIELD_SEPARATORS);
  addInParameter<string>(FIELD_SEPARATOR_CUSTOM, paramHelp[4], CUSTOM_MARK);
  addInParameter<StringCollection>(STRING_DELIMITER, paramHelp[5], STRING_DELIMITERS);
}

//================================================================================
bool CsvExport::exportGraph(std::ostream &os) {
  // initialize parameters with default values
  // only nodes are exported
  StringCollection eltTypes(ELT_TYPES);
  int eltType = 0;
  bool first = true;
  eltTypes.setCurrent(0);
  // export all
  bool exportSelection = false;
  // ids are not exported
  bool exportId = false;
  // export visual properties
  bool exportVisualProperties = false;
  // field separator is Custom
  StringCollection fieldSeparators(FIELD_SEPARATORS);
  fieldSeparators.setCurrent(0);
  // custom field separator is ;
  string fieldSeparatorCustom(CUSTOM_MARK);
  // string delimiter is "
  StringCollection stringDelimiters(STRING_DELIMITERS);
  stringDelimiters.setCurrent(0);

  // get choosen values of plugin parameters
  if (dataSet != NULL) {
    if (dataSet->get(ELT_TYPE, eltTypes))
      eltType = eltTypes.getCurrent();

    dataSet->get(EXPORT_SELECTION, exportSelection);
    dataSet->get(EXPORT_ID, exportId);
    dataSet->get(EXPORT_VISUAL_PROPERTIES, exportVisualProperties);
    dataSet->get(FIELD_SEPARATOR_CUSTOM, fieldSeparatorCustom);

    if (dataSet->get(FIELD_SEPARATOR, fieldSeparators)) {
      switch (fieldSeparators.getCurrent()) {
      case COMMA_SEPARATOR:
        fieldSeparator = ',';
        break;

      case TAB_SEPARATOR:
        fieldSeparator = '\t';
        break;

      case SPACE_SEPARATOR:
        fieldSeparator = ' ';
        break;

      case SEMICOLON_SEPARATOR:
        fieldSeparator = ';';

      default:
        fieldSeparator = fieldSeparatorCustom;
      }

    }

    if (dataSet->get(STRING_DELIMITER, stringDelimiters))
      stringDelimiter = stringDelimiters.getCurrent() == DBL_QUOTE_DELIMITER ? '"' : '\'';
  }

  // export names of fields
  // export ids if needed
  if (exportId) {
    if (eltType != EDGE_TYPE) {
      exportString(os, string("node id"));
    }

    if (eltType == BOTH_TYPES) {
      os << fieldSeparator;
    }

    if (eltType != NODE_TYPE) {
      exportString(os, string("src id"));
      os << fieldSeparator;
      exportString(os, string("tgt id"));
    }

    first = false;
  }

  // export non tulip defined properties
  Iterator<PropertyInterface *> *it(graph->getObjectProperties());
  // use vectors for further access to exported properties
  vector<PropertyInterface *> props;
  vector<bool> propIsString;
  unsigned int nbProps = 0;

  while(it->hasNext()) {
    PropertyInterface* prop = it->next();
    const string& propName = prop->getName();

    if (propName.compare(0, 4, "view") != 0 || exportVisualProperties) {
      ++nbProps;
      props.push_back(prop);
      propIsString.push_back(dynamic_cast<tlp::StringProperty*>(prop));

      if (!first)
        os << fieldSeparator;
      else
        first = false;

      exportString(os, propName);
    }
  }

  delete it;
  os << endl;

  // export nodes
  BooleanProperty* prop = graph->getProperty<BooleanProperty>("viewSelection");

  if(exportSelection && dataSet != NULL) {
    dataSet->get("Export selection property", prop);
  }

  if (eltType != EDGE_TYPE) {
    Iterator<node>* it = exportSelection ? prop->getNodesEqualTo(true, graph) : graph->getNodes();

    while(it->hasNext()) {
      node n = it->next();

      if (exportId) {
        os << n ;

        if (eltType == BOTH_TYPES)
          os << fieldSeparator << fieldSeparator;

        if (nbProps>0)
          os << fieldSeparator;
      }

      for (unsigned int i = 0; i < nbProps; ++i) {
        PropertyInterface *prop = props[i];
        string value = prop->getNodeStringValue(n);

        if (!value.empty()) {
          if (propIsString[i])
            exportString(os, value);
          else
            os << value;
        }

        if (i != nbProps - 1)
          os << fieldSeparator;
      }

      os << endl;
    }

    delete it;
  }

  // export edges
  if (eltType != NODE_TYPE) {
    Iterator<edge>* it =
      exportSelection ? prop->getEdgesEqualTo(true, graph) : graph->getEdges();

    while(it->hasNext()) {
      edge e = it->next();

      if (exportId) {
        if (eltType == BOTH_TYPES)
          os << fieldSeparator;

        const std::pair<node, node>& ends = graph->ends(e);
        os << ends.first << fieldSeparator << ends.second.id;

        if (nbProps>0)
          os << fieldSeparator;
      }

      for (unsigned int i = 0; i < nbProps; ++i) {
        PropertyInterface *prop = props[i];
        string value = prop->getEdgeStringValue(e);

        if (!value.empty()) {
          if (propIsString[i])
            exportString(os, value);
          else
            os << value;
        }

        if (i != nbProps - 1)
          os << fieldSeparator;
      }

      os << endl;
    }

    delete it;
  }

  return true;
}