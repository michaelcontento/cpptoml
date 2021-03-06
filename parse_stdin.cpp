#include "cpptoml.h"

#include <iostream>
#include <limits>

std::string escape_string( const std::string & str ) {
    std::string res;
    for( auto it = str.begin(); it != str.end(); ++it ) {
        if( *it == '\\' ) 
            res += "\\\\";
        else if( *it == '"' )
            res += "\\\"";
        else
            res += *it;
    }
    return res;
}

void print_value( std::ostream & o, const std::shared_ptr<cpptoml::toml_base> & base ) {
    if( auto v = base->as<std::string>() ) {
        o << "{\"type\":\"string\",\"value\":\"" << escape_string( v->value() ) << "\"}";
    } else if( auto v = base->as<int64_t>() ) {
        o << "{\"type\":\"integer\",\"value\":\"" << v->value() << "\"}";
    } else if( auto v = base->as<double>() ) {
        o << "{\"type\":\"float\",\"value\":\"" << v->value() << "\"}";
    } else if( auto v = base->as<std::tm>() ) {
        o << "{\"type\":\"datetime\",\"value\":\"";
#if CPPTOML_HAS_STD_PUT_TIME
        o << std::put_time( &v->value(), "%Y-%m-%dT%H:%M:%SZ" ) << "\"}";
#else
        std::array<char, 100> buf;
        if( std::strftime( &buf[0], 100, "%Y-%m-%dT%H:%M:%SZ", &v->value() ) )
            o << &buf[0];
#endif
    } else if( auto v = base->as<bool>() ) {
        o << "{\"type\":\"bool\",\"value\":\"";
        v->print( o );
        o << "\"}";
    } else if( auto v = base->as<std::vector<std::shared_ptr<cpptoml::toml_base>>>() ) {
        o << "{\"type\":\"array\",\"value\":[";
        auto it = v->value().begin();
        while( it != v->value().end() ) {
            print_value( o, *it );
            if( ++it != v->value().end() )
                o << ", ";
        }
        o << "]}";
    }
}

void print_group( std::ostream & o, cpptoml::toml_group & g ) {
    o << "{";
    auto it = g.begin();
    while( it != g.end() ) {
        o << '"' << escape_string( it->first ) << "\":";
        if( it->second->is_group() ) {
            print_group( o, *g.get_group( it->first ) );
        } else {
            print_value( o, it->second );
        }
        if( ++it != g.end() )
            o << ", ";
    }
    o << "}";
}

int main() {
    std::cout.precision( std::numeric_limits<double>::max_digits10 );
    cpptoml::parser p{ std::cin };
    try {
        cpptoml::toml_group g = p.parse();
        print_group( std::cout, g );
        std::cout << std::endl;
    } catch( ... ) {
        return 1;
    }
    return 0;
}
