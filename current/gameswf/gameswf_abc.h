// gameswf_abc.h	-- Vitaly Alexeev <tishka92@yahoo.com>	2008

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

// do_abc tag reader

#ifndef GAMESWF_ABC_H
#define GAMESWF_ABC_H

#include "gameswf/gameswf_types.h"
#include "gameswf/gameswf_avm2.h"
#include "gameswf/gameswf_log.h"

#include <map>

namespace gameswf
{

	struct abc_def;
	struct as_function;
	struct movie_definition_sub;

	struct multiname
	{
		enum kind
		{
			CONSTANT_UNDEFINED = 0,
			CONSTANT_QName = 0x07,
			CONSTANT_QNameA = 0x0D,
			CONSTANT_RTQName = 0x0F,
			CONSTANT_RTQNameA = 0x10,
			CONSTANT_RTQNameL = 0x11,
			CONSTANT_RTQNameLA = 0x12,
			CONSTANT_Multiname = 0x09,
			CONSTANT_MultinameA = 0x0E,
			CONSTANT_MultinameL = 0x1B,
			CONSTANT_MultinameLA = 0x1C
		};

		int m_kind;
		int m_flags;
		int m_ns;
		int m_ns_set;
		int m_name;

		multiname() :
			m_kind(CONSTANT_UNDEFINED),
			m_flags(0),
			m_ns(0),
			m_name(0)
		{
		}

		inline bool is_qname() const
		{
			return m_kind == CONSTANT_QName;
		}
	};

	struct as3_namespace
	{
        struct kind
        {
            enum Enum
            {
                CONSTANT_Undefined = 0,
                CONSTANT_Namespace = 0x08,
                CONSTANT_PackageNamespace = 0x16,
                CONSTANT_PackageInternalNs = 0x17,
                CONSTANT_ProtectedNamespace = 0x18,
                CONSTANT_ExplicitNamespace = 0x19,
                CONSTANT_StaticProtectedNs = 0x1A,
                CONSTANT_PrivateNs = 0x05
            };

            static lfl_string to_string(Enum e)
            {
                switch(e)
                {
                case CONSTANT_Undefined:
                    return "CONSTANT_Undefined";
                    break;

                case CONSTANT_Namespace:
                    return "CONSTANT_Namespace";
                    break;

                case  CONSTANT_PackageNamespace:
                    return "CONSTANT_PackageNamespace";
                    break;

                case  CONSTANT_PackageInternalNs:
                    return "CONSTANT_PackageInternalNs";
                    break;

                case CONSTANT_ProtectedNamespace:
                    return "CONSTANT_ProtectedNamespace";
                    break;

                case CONSTANT_ExplicitNamespace:
                    return "CONSTANT_ExplicitNamespace";
                    break;

                case CONSTANT_StaticProtectedNs:
                    return "CONSTANT_StaticProtectedNs";
                    break;

                case CONSTANT_PrivateNs:
                    return "CONSTANT_PrivateNs";
                    break;

                default:
                    assert("Default section in switch" && false);
                    return "";
                }
            }
        };

        kind::Enum  m_kind;
        int         m_name;

        as3_namespace() :
        m_kind(kind::CONSTANT_Undefined),
            m_name(0)
        {
        }

    };

    /*
        The metadata_info entry provides a means of embedding arbitrary key /value pairs into the ABC file. The 
        AVM2 will ignore all such entries.  
    */

	struct metadata_info : public ref_counted
	{
        typedef int key_t;
        typedef int value_t;

        int     m_name;
        int     m_item_count;

        //std::map<key_t, value_t> m_items_map;
        array<std::pair<key_t, value_t>> m_items;

        void    read(lfl_stream* in, abc_def* abc);
    };


	//
	// instance_info
	//
	struct instance_info : public ref_counted
	{

		enum flags
		{
			CONSTANT_ClassSealed = 0x01,
			CONSTANT_ClassFinal = 0x02,
			CONSTANT_ClassInterface = 0x04,
			CONSTANT_ClassProtectedNs = 0x08
		};

		int m_name;
		int m_super_name;
		Uint8 m_flags;
		int m_protectedNs;
		array<int> m_interface;
		int m_iinit;
		gc_array<gc_ptr<traits_info> > m_trait;
		weak_ptr<abc_def> m_abc;

		instance_info() :
			m_name(0),
			m_super_name(0),
			m_flags(0),
			m_protectedNs(0),
			m_iinit(0)
		{
		}

		void	read(lfl_stream* in, abc_def* abc);
	};

	struct class_info : public ref_counted
	{
		weak_ptr<abc_def> m_abc;
		int m_cinit;
		gc_array<gc_ptr<traits_info> > m_trait;

		void	read(lfl_stream* in, abc_def* abc);
	};

	struct script_info : public ref_counted
	{
		int m_init;
		gc_array<gc_ptr<traits_info> > m_trait;

		void	read(lfl_stream* in, abc_def* abc);
	};

	struct abc_def : public ref_counted
	{
		// constant pool
		array<int> m_integer;
		array<Uint32> m_uinteger;
		array<double> m_double;
		array<lfl_string> m_string;
		array<as3_namespace> m_namespace;
		array< array<int> > m_ns_set;
		array<multiname> m_multiname;

//		array<gc_ptr<method_info> > m_method;
		gc_array<gc_ptr<as_3_function> > m_method;
		gc_array<gc_ptr<metadata_info> > m_metadata;
		gc_array<gc_ptr<instance_info> > m_instance;
		gc_array<gc_ptr<class_info> > m_class;
		gc_array<gc_ptr<script_info> > m_script;

		inline const char* get_string(int index) const
		{
			return m_string[index].c_str(); 
		}

		inline int get_integer(int index) const
		{
			return m_integer[index]; 
		}

		inline double get_double(int index) const
		{
			return m_double[index]; 
		}

		inline const char* get_namespace(int index) const
		{
			return get_string(m_namespace[index].m_name); 
		}

		inline const char* get_multiname(int index) const
		{
			return get_string(m_multiname[index].m_name); 
		}

		inline multiname::kind get_multiname_type(int index) const
		{
			return (multiname::kind)m_multiname[index].m_kind; 
		}

		inline const char* get_multiname_namespace(int index) const
		{
			const multiname & mn = m_multiname[index];

			switch( mn.m_kind )
			{
				case multiname::CONSTANT_QName:
					return "";
				case multiname::CONSTANT_Multiname:
				case multiname::CONSTANT_MultinameA:
					return get_namespace( mn.m_ns );
				default:
					log_msg( "implement get_multiname_namespace for this kind %i\n", mn.m_kind );
					return NULL;
			} 
		}

		as_function* get_script_function( const lfl_string & name = "" ) const;

		inline as_function* get_class_function( const int class_index ) const
		{
			return m_method[m_class[class_index]->m_cinit].get();
		}

		abc_def(player* player);
		virtual ~abc_def();

		void	read(lfl_stream* in, movie_definition_sub* m);
		void	read_cpool(lfl_stream* in);

		inline const char * get_super_class(lfl_string& name) const
		{
			return get_multiname( get_instance_info( name )->m_super_name );
		}

		const char * get_class_from_constructor(int method);

		// get class constructor
		as_function* get_class_constructor(const lfl_string& name) const;

		// find instance info by name
		instance_info* get_instance_info(const lfl_string& class_name) const;
		class_info* get_class_info(const lfl_string& full_class_name) const;
		class_info* get_class_info(int class_index) const;

        const lfl_string& get_method_name(int mtid) const;
	};
}


#endif // GAMESWF_ABC_H
