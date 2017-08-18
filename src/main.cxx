#include <algorithm>
#include <array>
#include <cstdlib>
#include <iostream>
#include <iterator>
#include <regex>
#include <sstream>
#include <string>
#include <vector>


#include <tao/pegtl.hpp>

using namespace tao::TAOCPP_PEGTL_NAMESPACE;

struct Field {
  std::string name;
  std::string type;
  std::string value;
};

struct Event {
  int parsed;
  Event() : parsed(0) {}
  std::string table;
  std::string op;
  std::vector<Field> fields;

  std::string json() {
    if (parsed == 0) {
      return nullptr;
    }
    std::ostringstream ss;
    std::vector<std::string> items;
    for (auto field : fields) {
      std::string value { std::regex_replace(field.value, std::regex("\""), "\\\"") };
      std::replace(value.begin(), value.end(), '\'', '"');
      items.push_back("    \"" + field.name + "\": " + value);
    }
    std::copy(items.begin(), items.end() - 1, std::ostream_iterator<std::string>(ss, ",\n"));
    ss << items.back();
    std::string data = ss.str();
    return
      "{\n"
      "  \"table\": \""     + table + "\",\n"
      "  \"operation\": \"" + op    + "\",\n"
      "  \"data\": {\n"     + data  + "\n"
      "  }\n"
      "}";
  }
};

namespace logs {

  struct sql_op : sor< string<'U','P','D','A','T','E'>,
		       string<'I','N','S','E','R','T'>,
		       string<'D','E','L','E','T','E'> > {};
  
  struct word    : star< sor< alnum, one<'_'> > > {};
  struct ns      : star< sor< alnum, one<'_'>, one<'.'> > > {};
  struct begin   : seq< string<'B','E','G','I','N'>, star<any>, eolf >  {};
  struct commit  : seq< string< 'C','O','M','M','I','T'>, star<any> > {};
  struct colon   : one<':'> {};
  struct lbrace  : one<'['> {};
  struct rbrace  : one<']'> {};
  struct squote  : one<'\''> {};
  struct arrtype : string<'[',']'> {};

  struct schema  : word {};
  struct table   : word {};
  struct colname : word {};
  struct coltype : seq< list<ns, blank>, opt<arrtype> > {};
  
  struct quoted  : seq< squote, until<squote, any> > {};
  struct unquoted: until<at<sor<blank, eof, eolf>>, any> {};
  struct value   : sor< quoted, unquoted > {};

  struct field   : seq< colname, lbrace, coltype, rbrace, colon, value > {};
  struct logline : seq< string< 't','a','b','l','e' >,
			blank,
			schema,
			one<'.'>,
			table,
			colon,	   
			blank,
			sql_op,
			colon,
			blank,
			list<field, blank>,
			eolf > {};

  struct grammar : sor< logline, begin, commit > {};

  template< typename Rule >
  struct action : nothing< Rule > {};


  
  template<>
  struct action< table > {
    template< typename Input >
    static void apply( const Input& in, Event& record ) {
      record.parsed = 1;
      record.table = in.string();
    }
  };
  
  template<>
  struct action< sql_op > {
    template< typename Input >
    static void apply( const Input& in, Event& record ) {
      record.op = in.string();
    }
  };

  template<>
  struct action< colname > {
    template< typename Input >
    static void apply( const Input& in, Event& record ) {
      Field field;
      field.name = in.string();
      record.fields.push_back(field);
    }
  };

  template<>
  struct action< coltype > {
    template< typename Input >
    static void apply( const Input& in, Event& record ) {
      record.fields.back().type = in.string();
    }
  };

  
  template<>
  struct action< value > {
    template< typename Input >
    static void apply( const Input& in, Event& record ) {
      record.fields.back().value = in.string();
    }
  };
  
}

int main() {
  std::string str;
  try {
    while( !std::getline( std::cin, str ).fail() ) {
      Event record;
      memory_input<> in( str, "std::cin" );
      if( parse< logs::grammar, logs::action >( in, record ) ) {
	if (record.parsed)
	  std::cout << record.json() << std::endl;
      } else {
	std::cout << "parsing failed" << std::endl;
      }
    }
  } catch (const std::exception& e) {
    std::cout << e.what() << std::endl;
  }
}
