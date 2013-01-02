// gameswf_avm2.cpp	-- Vitaly Alexeev <tishka92@yahoo.com>	2008

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

// AVM2 implementation

#include "gameswf/gameswf_avm2.h"
#include "gameswf/gameswf_stream.h"
#include "gameswf/gameswf_log.h"
#include "gameswf/gameswf_abc.h"
#include "gameswf/gameswf_disasm.h"
#include "gameswf/gameswf_character.h"
#include "gameswf_jit.h"
#include "gameswf/gameswf_as_classes/as_array.h"
#include "gameswf/gameswf_as_classes/as_class.h"

#include "gameswf/lfl_as_opcodes.h"

namespace gameswf
{

    as_3_function::as_3_function(abc_def* abc, int method, player* player) :
as_function(player),
    m_abc(abc),
    m_return_type( -1 ),
    m_name( -1 ),
    m_flags( 0 ),
    m_method(method),
    m_max_stack( 0 ),
    m_local_count( 0 ),
    m_init_scope_depth( 0 ),
    m_max_scope_depth( 0 )
{
    m_this_ptr = this;

    // any function MUST have prototype
    builtin_member("prototype", new as_object(player));
}

as_3_function::~as_3_function()
{
}

void	as_3_function::operator()(const function_call& fn)
    // dispatch
{
    assert(fn.env);

    // try to use caller environment
    // if the caller object has own environment then we use its environment
    as_environment* env = fn.env;
    if (fn.this_ptr)
    {
        if (fn.this_ptr->get_environment())
        {
            env = fn.this_ptr->get_environment();
        }
    }

    // set 'this'
    as_object* this_ptr = env->get_target();
    if (fn.this_ptr)
    {
        this_ptr = fn.this_ptr;
        if (this_ptr->m_this_ptr != NULL)
        {
            this_ptr = this_ptr->m_this_ptr.get_ptr();
        }
    }

    // Create local registers.
    array<as_value>	local_register;
    local_register.resize(m_local_count + 1);

    // Register 0 holds the ?this? object. This value is never null.
    assert(this_ptr);
    local_register[0] = this_ptr;

    // Registers 1 through method_info.param_count holds parameter values.
    // If fewer than method_body_info.local_count values are supplied to the call then
    // the remaining values are either the values provided by default value declarations 
    // or the value undefined.
    for (int i = 0; i < m_param_type.size(); i++)
    {
        // A zero value denotes the any (?*?) type.
        //const char* name = m_abc->get_multiname(m_param_type[i]);
        local_register[i + 1] = fn.arg( i );	// hack
    }

#ifdef __GAMESWF_ENABLE_JIT__

    if( !m_compiled_code.is_valid() )
    {
        compile();
        m_compiled_code.initialize();
    }

#endif

    if( m_compiled_code.is_valid() )
    {
        try
        {
            m_compiled_code.call< array<as_value>&, vm_stack&, vm_stack&, as_value* >
                (local_register, *env, env->m_scope, fn.result );
        }
        catch( ... )
        {
            log_msg( "jitted code crashed" );
        }
    }
    else
    {
        // keep stack size on entry
        int stack_size = env->size();
        const lfl_string& mname = m_abc->get_method_name(m_method);
        IF_VERBOSE_ACTION(log_msg("\nEX: call method #%d. Method name [%s] \n", m_method, mname.c_str()));

        // Execute the actions.
        execute(local_register, env, fn.result);

        IF_VERBOSE_ACTION(log_msg("EX: ended #%d.\n\n", m_method));

        if (stack_size != env->size())
        {
            log_error("error: stack size on exit must be same as on entry, %d:%d \n",
                stack_size, env->size());

            // restore stack size
            env->resize(stack_size);
        }
    }

}

// interperate action script bytecode
void	as_3_function::execute(array<as_value>& lregister, as_environment* env, as_value* result)
{
    // m_abc may be destroyed
    assert(m_abc != NULL);

    vm_stack& stack = *env;
    vm_stack& scope = env->m_scope;

    // some method have no body
    if (m_code.size() == 0)
    {
        return;
    }

    IF_VERBOSE_PARSE(log_msg("--->>>\n"));
    IF_VERBOSE_PARSE(log_disasm_avm2(m_code, m_abc.get_ptr()));
    IF_VERBOSE_PARSE(log_msg("<<<---\n"));

    int ip = 0;
    do
    {
        Uint8 opcode = m_code[ip++];
        switch (opcode)
        {
        case AS_OPCODE_IfTrue: // iftrue
            {
                bool taken;
                //Follows ECMA-262 11.9.3
                taken = stack.top(0).to_bool();
                stack.drop(1);
                if (taken)
                {
                    int offset = read_signed_offset_24(m_code, ip);
                    ip += offset;
                }

                ip += 3;

                IF_VERBOSE_ACTION(log_msg("EX: iftrue\t %s\n", taken? "taken": "not taken"));
            }
            break;


        //Branch if the first value is not greater than or equal to the second value.
        case AS_OPCODE_IfNotGreaterThanOrEqual: //ifnge
            {
                as_value& v2 = stack.top(0);
                as_value& v1 = stack.top(1);

                //IF_VERBOSE_ACTION(log_msg("if (%f < %f)", v1.to_number(), v2.to_number()));

                as_value taken;
                bool is_nan = as_value::abstract_relational_comparison(v1, v2, taken);
                UNUSED(is_nan);

                bool val = taken.to_bool();
                if (val)
                {
                    int offset = read_signed_offset_24(m_code, ip);
                    ip += offset;
                }

                stack.drop(2);

                ip += 3;
                IF_VERBOSE_ACTION(log_msg("EX: ifnge\t %s\n", val? "taken": "not taken"));
            }
            break;

        //Branch if the first value is less than the second value.
        case AS_OPCODE_IfLessThan: //iflt
            {
                as_value& v2 = stack.top(0);
                as_value& v1 = stack.top(1);

                //IF_VERBOSE_ACTION(log_msg("if (%f < %f)", v1.to_number(), v2.to_number()));
                as_value taken;
                bool is_nan = as_value::abstract_relational_comparison(v1, v2, taken);
                bool val = taken.to_bool();
                if (val)
                {
                    int offset = read_signed_offset_24(m_code, ip);
                    ip += offset;
                }

                stack.drop(2);

                ip += 3;
                IF_VERBOSE_ACTION(log_msg("EX: iflt\t %s\n", val? "taken": "not taken"));
            }
            break;

        //Branch if the first value is less than or equal to the second value.
        case AS_OPCODE_IfLessThanOrEqual: //ifle
            {
                as_value& v2 = stack.top(0);
                as_value& v1 = stack.top(1);

                //IF_VERBOSE_ACTION(log_msg("if (%f < %f)", v1.to_number(), v2.to_number()));
                as_value less_val;
                bool is_nan = as_value::abstract_relational_comparison(v2, v1, less_val);
                UNUSED(is_nan);
                bool less = less_val.to_bool();

                if (!less) // v1 <= v2 is same v2 > v1
                {
                    int offset = read_signed_offset_24(m_code, ip);
                    ip += offset;
                }

                stack.drop(2);

                ip += 3;
                IF_VERBOSE_ACTION(log_msg("EX: ifle\t %s\n", less? "not taken": "taken"));
            }
            break;

        //Branch if the first value is greater than the second value.
        case AS_OPCODE_IfGreaterThan: //ifgt
            {
                //Follows ECMA-262 11.9.3

                as_value& v2 = stack.top(0);
                as_value& v1 = stack.top(1);

                //verbose numbers only 
                //IF_VERBOSE_ACTION(log_msg("if (%f > %f)", v2.to_number(), v1.to_number()));
                as_value taken;
                bool is_nan = as_value::abstract_relational_comparison(v2, v1, taken);
                UNUSED(is_nan);
                bool val = taken.to_bool();
                if (val)
                {
                    int offset = read_signed_offset_24(m_code, ip);
                    ip += offset;
                }

                stack.drop(2);

                ip += 3;
                IF_VERBOSE_ACTION(log_msg("EX: ifgt\t %s\n", val? "taken": "not taken"));
            }
            break;

        //Branch if the first value is greater than or equal to the second value.
        case AS_OPCODE_IfGreaterThanOrEqual: //ifge
            {
                //Follows ECMA-262 11.9.3
                as_value& v2 = stack.top(0);
                as_value& v1 = stack.top(1);

                as_value taken;
                bool is_nan = as_value::abstract_relational_comparison(v1, v2, taken);
                UNUSED(is_nan);
                bool val = taken.to_bool();

                if (!val)
                {
                    int offset = read_signed_offset_24(m_code, ip);
                    ip += offset;
                }

                stack.drop(2);

                ip += 3;
                IF_VERBOSE_ACTION(log_msg("EX: ifge\t %s\n", val? "not taken": "taken"));
            }
            break;

        //Branch if the first value is not less than the second value.
        case AS_OPCODE_IfNotLessThan: //ifnlt
            {
                //Follows ECMA-262 11.9.3
                as_value& v2 = stack.top(0);
                as_value& v1 = stack.top(1);

                as_value taken;
                bool is_nan = as_value::abstract_relational_comparison(v1, v2, taken);
                UNUSED(is_nan);

                bool val = taken.to_bool();

                if (!val)
                {
                    int offset = read_signed_offset_24(m_code, ip);
                    ip += offset;
                }

                stack.drop(2);

                ip += 3;
                IF_VERBOSE_ACTION(log_msg("EX: ifnlt\t %s\n", val? "not taken": "taken"));
            }
            break;

        //Branch if the first value is not less than or equal to the second value.
        case AS_OPCODE_IfNotLessThanOrEqual: //ifnle
            {
                //Follows ECMA-262 11.9.3
                as_value& v2 = stack.top(0);
                as_value& v1 = stack.top(1);

                as_value taken;
                bool is_nan = as_value::abstract_relational_comparison(v2, v1, taken);
                bool val = taken.to_bool();

                if (val)
                {
                    int offset = read_signed_offset_24(m_code, ip);
                    ip += offset;
                }

                stack.drop(2);

                ip += 3;
                IF_VERBOSE_ACTION(log_msg("EX: ifnle\t %s\n", val? "taken": "not taken"));
            }
            break;

        //Branch if the first value is not greater than the second value.
        case AS_OPCODE_NotGreaterThan: //ifngt
            {
                //Follows ECMA-262 11.9.3
                as_value& v2 = stack.top(0);
                as_value& v1 = stack.top(1);

                as_value taken;
                bool is_nan = as_value::abstract_relational_comparison(v2, v1, taken);
                UNUSED(is_nan);
                bool val = taken.to_bool();

                if (!val)
                {
                    int offset = read_signed_offset_24(m_code, ip);
                    ip += offset;
                }

                stack.drop(2);

                ip += 3;
                IF_VERBOSE_ACTION(log_msg("EX: ifnle\t %s\n", val? "not taken": "taken"));
            }
            break;

        case AS_OPCODE_GreaterThan: //greaterthan
            {
                as_value& v2 = stack.top(0);
                as_value& v1 = stack.top(1);

                as_value taken;
                bool is_nan = as_value::abstract_relational_comparison(v2, v1, taken);
                UNUSED(is_nan);
                stack.drop(2);

                bool val = taken.to_bool();
                stack.push(val);

                IF_VERBOSE_ACTION(log_msg("EX: greaterthan\t %s\n", val? "taken": "not taken"));
            }
            break;

        case AS_OPCODE_GreaterThanOrEqual: //greaterequals
            {
                as_value& v2 = stack.top(0);
                as_value& v1 = stack.top(1);

                as_value taken;
                bool is_nan = as_value::abstract_relational_comparison(v1, v2, taken);
                UNUSED(is_nan);
                stack.drop(2);

                bool val = !taken.to_bool();
                stack.push(val);

                IF_VERBOSE_ACTION(log_msg("EX: greaterequals\t %s\n", val? "taken": "not taken"));
            }
            break;

        case AS_OPCODE_IfFalse: // iffalse
            {
                bool taken;
                //Follows ECMA-262 11.9.3
                taken = !stack.top(0).to_bool();
                stack.drop(1);
                if (taken)
                {
                    int offset = read_signed_offset_24(m_code, ip);
                    ip += offset;
                }

                ip += 3;
                IF_VERBOSE_ACTION(log_msg("EX: iffalse\t %s\n", taken? "taken": "not taken"));
            } break;

        case AS_OPCODE_IfEqual: //ifeq
            {
                bool taken;
                //Follows ECMA-262 11.9.3
                taken = as_value::abstract_equality_comparison(scope[ scope.size() - 2 ], scope[ scope.size() - 1 ]);
                if (taken)
                {
                    int offset = read_signed_offset_24(m_code, ip);
                    ip += offset;
                }

                ip += 3;
                IF_VERBOSE_ACTION(log_msg("EX: ifne\t %s\n", taken? "taken": "not taken"));
            }
            break;

        case AS_OPCODE_IfNotEqual: // ifne
            {
                bool taken;
                //Follows ECMA-262 11.9.3
                taken = !as_value::abstract_equality_comparison(scope[ scope.size() - 2 ], scope[ scope.size() - 1 ]);
                if (taken)
                {
                    int offset = read_signed_offset_24(m_code, ip);
                    ip += offset;
                }

                ip += 3;
                IF_VERBOSE_ACTION(log_msg("EX: ifne\t %s\n", taken? "taken": "not taken"));

            } break;

        case AS_OPCODE_PopScope: // popscope
            {
                scope.pop();
                IF_VERBOSE_ACTION(log_msg("EX: popscope\n"));
            } break;

        case AS_OPCODE_PushNull:  // pushnull
            {
                as_value value;
                value.set_null();
                stack.push( value );
                IF_VERBOSE_ACTION(log_msg("EX: pushnull\n"));
            } break;

        case AS_OPCODE_PushByte:	// pushbyte
            {
                int byte_value;
                ip += read_vu30(byte_value, &m_code[ip]);
                stack.push(byte_value);

                IF_VERBOSE_ACTION(log_msg("EX: pushbyte\t %d\n", byte_value));
            } break;

        case AS_OPCODE_PushShort:  // pushshort
            {
                int val;
                ip += read_vu30(val, &m_code[ip]);
                stack.push(val);
                IF_VERBOSE_ACTION(log_msg("EX: pushshort\t %d\n", val));
            } break;

        case AS_OPCODE_PushTrue:  // pushtrue
            {
                stack.push( true );
                IF_VERBOSE_ACTION(log_msg("EX: pushtrue\n"));
            } break;

        case AS_OPCODE_PushFalse:  // pushfalse
            {
                stack.push( false );

                IF_VERBOSE_ACTION(log_msg("EX: pushfalse\n"));
            }
            break;

        case AS_OPCODE_PopAndDiscard:  // pop the value from stack and discard it
            {
                stack.pop();
                IF_VERBOSE_ACTION(log_msg("EX: pop\n"));
                break;
            }

        case AS_OPCODE_Dup:  // dup
            {
                IF_VERBOSE_ACTION(log_msg("EX: dup %s\n", stack.top(0).to_xstring()));
                stack.push(stack.top(0));
            } break;

        case AS_OPCODE_PushInt:	// pushint
            {
                int index;
                ip += read_vu30(index, &m_code[ip]);
                int val = m_abc->get_integer(index);
                stack.push(val);

                IF_VERBOSE_ACTION(log_msg("EX: pushint\t %d\n", val));

                break;
            }

        case AS_OPCODE_PushString:	// pushstring
            {
                int index;
                ip += read_vu30(index, &m_code[ip]);
                const lfl_string& val = m_abc->get_string(index);
                stack.push(val);

                IF_VERBOSE_ACTION(log_msg("EX: pushstring\t '%s'\n", val));

                break;
            }

        case AS_OPCODE_PushDouble:	// pushdouble
            {
                int index;
                ip += read_vu30(index, &m_code[ip]);
                double val = m_abc->get_double(index);
                stack.push(val);

                IF_VERBOSE_ACTION(log_msg("EX: pushdouble\t %f\n", val));

                break;
            }

        case AS_OPCODE_PushScope:	// pushscope
            {
                as_value val = stack.pop();
                scope.push(val);

                IF_VERBOSE_ACTION(log_msg("EX: pushscope\t %s\n", val.to_xstring()));

                break;
            }

        case AS_OPCODE_CallProperty:  // callproperty
            {
                int index;
                ip += read_vu30(index, &m_code[ip]);
                const lfl_string& name = m_abc->get_multiname(index);

                int arg_count;
                ip += read_vu30(arg_count, &m_code[ip]);

                as_environment env(get_player());
                for (int i = 0; i < arg_count; i++)
                {
                    env.push(stack.top(i));
                }
                stack.drop(arg_count);

                as_value result;

                if( stack.top(0).is_object() )
                {
                    as_object* obj = stack.top(0).to_object();

                    as_value func, func2;

                    result.set_undefined();

                    if( obj &&  obj->get_member(name, &func))
                    {
                        if( func.is_function() )
                        {
                            result = call_method(func, &env, obj, arg_count, env.get_top_index()); 
                        }
                        else if(func.to_object()->get_member( "__call__", &func2 ) )
                        {
                            //todo patch scope
                            result = call_method(func2, &env, obj, arg_count, env.get_top_index());
                        }
                    }
                }
                else
                {
                    as_value func;
                    if( stack.top(0).find_property( name, &func ) )
                    {
                        result = call_method(func, &env, stack.top(0), arg_count, env.get_top_index()); 
                    }
                }

                IF_VERBOSE_ACTION(log_msg("EX: callproperty\t 0x%p.%s(args:%d), result %s\n", stack.top(0).to_xstring(), name.c_str(),
                    arg_count, result.to_xstring()));

                stack.drop(1);
                stack.push( result );

            } break;

        case AS_OPCODE_ReturnVoid:	// returnvoid
            {
                IF_VERBOSE_ACTION(log_msg("EX: returnvoid\t\n"));
                result->set_undefined();
                return;
            }

        case AS_OPCODE_ReturnValue:	// returnvalue
            {
                IF_VERBOSE_ACTION(log_msg("EX: returnvalue \t%s\n", stack.top(0).to_xstring()));
                *result = stack.pop();
                return;
            }

        case AS_OPCODE_ConstructSuper:	// constructsuper
            {
                // stack: object, arg1, arg2, ..., argn
                int arg_count;
                ip += read_vu30(arg_count, &m_code[ip]);

                as_environment env(get_player());
                for (int i = 0; i < arg_count; i++)
                {
                    env.push(stack.pop());
                }

                gc_ptr<as_object> obj = stack.pop().to_object();

                // Assume we are in a constructor
                lfl_string class_name = m_abc->get_class_from_constructor( m_method );
                lfl_string super_class_name = m_abc->get_super_class( class_name );


                as_object * super = obj.get_ptr();

                while( super->get_proto() )
                {
                    super = super->get_proto();
                }

                as_function * function = m_abc->get_class_constructor( super_class_name );
                if( !function )
                {
                    as_value value;
                    player* pl = get_player();
                    as_object* glob = pl->get_global();
                    bool success = glob->get_member(super_class_name, &value);
                    if(success)
                    {
                        function = cast_to<as_function>( value.to_object() );
                    }
                }
                assert( function );
                as_object* proto = super->create_proto( function );
                UNUSED(proto);

                call_method( function, &env, obj.get_ptr(), arg_count, 0);

                //stack.top(0) = obj.get_ptr();

                IF_VERBOSE_ACTION(log_msg("EX: constructsuper\t 0x%p(args:%d)\n", obj.get_ptr(), arg_count));
            } break;

        case AS_OPCODE_ConstructProp: //constructprop
            {
                // Stack ..., obj, [ns], [name], arg1,...,argn => ..., value
                int index;
                ip += read_vu30(index, &m_code[ip]);
                const lfl_string& name = m_abc->get_multiname(index);
                const lfl_string& name_space = m_abc->get_multiname_namespace(index);
                UNUSED(name_space);

                int arg_count;
                ip += read_vu30(arg_count, &m_code[ip]);

                as_environment env(get_player());
                for (int i = 0; i < arg_count; i++)
                {
                    env.push(stack.top(i));
                }
                stack.drop(arg_count);

                as_object* obj = stack.pop().to_object();

                as_value func, func2;

                gc_ptr<as_object> new_object;
                if( obj && obj->get_member(name, &func))
                {
                    new_object = new as_object(get_player());
                    //:TODO: create prototype .... ( move instanciate class from character -> as_object )
                    new_object->set_instance( m_abc->get_instance_info( name ) );
                    call_method( m_abc->get_class_constructor( name ), &env, new_object.get_ptr(), arg_count, 0 );
                }

                IF_VERBOSE_ACTION(log_msg("EX: constructprop\t 0x%p.%s(args:%d)\n", obj, name.c_str(), arg_count));

                stack.push( new_object.get_ptr() );
            } break;

        case AS_OPCODE_ConstructPropVoid:	// callpropvoid, Call a property, discarding the return value.
            {
                // Stack: ..., obj, [ns], [name], arg1,...,argn => ...
                int index;
                ip += read_vu30(index, &m_code[ip]);
                const lfl_string& name = m_abc->get_multiname(index);

                int arg_count;
                ip += read_vu30(arg_count, &m_code[ip]);

                as_environment env(get_player());
                for (int i = 0; i < arg_count; i++)
                {
                    env.push(stack.top(i));
                }
                stack.drop(arg_count);

                as_object* obj = stack.pop().to_object();

                as_value func, func2;

                if( obj &&  obj->get_member(name, &func))
                {
                    if( func.is_function() )
                    {
                        call_method(func, &env, obj, arg_count, env.get_top_index());
                    }
                    else if(func.to_object()->get_member( "__call__", &func2 ) )
                    {
                        //todo patch scope
                        call_method(func2, &env, obj, arg_count, env.get_top_index());
                    }
                }

                IF_VERBOSE_ACTION(log_msg("EX: callpropvoid\t 0x%p.%s(args:%d)\n", obj, name.c_str(), arg_count));
            } break;

        case AS_OPCODE_NewArray: //newarray
            {
                int arg_count;

                ip += read_vu30(arg_count, &m_code[ip]);

                as_array * array = new as_array( get_player() );

                int offset = stack.size() - arg_count;

                for( int arg_index = 0; arg_index < arg_count; ++arg_index )
                {
                    array->push( stack[ offset + arg_index ] );
                }

                stack.resize( offset + 1 );
                stack.top(0) = array;

                IF_VERBOSE_ACTION(log_msg("EX: newarray\t arg_count:%i\n", arg_count));

            } break;

        case AS_OPCODE_NewClass: // newclass
            {
                // stack:	..., basetype => ..., newclass
                int class_index;
                ip += read_vu30( class_index, &m_code[ip] );

                IF_VERBOSE_ACTION(log_msg("EX: newclass\t class index:%i\n", class_index));

                //					as_object* basetype = stack.top(0).to_object();

                gc_ptr<as_class> new_class = new as_class(get_player());

                //new_class->set_proto(basetype);
                new_class->set_class(m_abc->get_class_info(class_index));

                as_environment env( get_player() );
                call_method( m_abc->get_class_function( class_index ), &env, new_class.get_ptr(), 0, 0 );

                stack.top(0).set_as_object(new_class.get_ptr());
            } break;

        case AS_OPCODE_FindPropStrict:	// findpropstrict
            {
                int index;
                ip += read_vu30(index, &m_code[ip]);
                const lfl_string& name = m_abc->get_multiname(index);

                // search property in scope
                as_object* obj = scope.find_property(name);

                //Search for a script entry to execute

                as_function* func = m_abc->get_script_function(name);
                if (obj == NULL && func != NULL)
                {
                    get_global()->set_member( name, new as_object( get_player() ) );

                    as_environment env( get_player() );

                    call_method( func, &env, get_global(), 0, 0 );

                    obj = get_global();
                }
                IF_VERBOSE_ACTION(log_msg("EX: findpropstrict\t %s, obj=0x%p\n", name.c_str(), obj));

                stack.push(obj);
            } break;

        case AS_OPCODE_FindProperty:	// findproperty, Search the scope stack for a property
            {
                int index;
                ip += read_vu30(index, &m_code[ip]);
                const lfl_string& name = m_abc->get_multiname(index);
                const lfl_string& name_space = m_abc->get_multiname_namespace(index);
                UNUSED(name_space);

                as_object* obj = scope.find_property(name);

                if( obj )
                {
                    IF_VERBOSE_ACTION(log_msg("EX: findproperty\t '%s', obj=0x%p\n", name.c_str(), obj));
                    stack.push(obj);
                }
                else
                {
                    IF_VERBOSE_ACTION(log_msg("EX: findproperty\t '%s', obj=global\n", name.c_str()));
                    stack.push(get_global());
                }
            } break;

        case AS_OPCODE_GetLex:	// getlex, Find and get a property.
            {
                int index;
                ip += read_vu30(index, &m_code[ip]);
                const lfl_string& name = m_abc->get_multiname(index);

                // search and get property in scope
                as_value val;
                scope.get_property(name, &val);

                if(val.is_undefined())
                {
                    as_function* func = m_abc->get_script_function(name);
                    if (func != NULL)
                    {
                        gc_ptr<as_object> object = new as_object( get_player() );
                        get_global()->set_member(name, object.get());

                        as_environment env( get_player() );

                        call_method( func, &env, get_global(), 0, 0 );

                        val.set_as_object(object);
                    }
                }

                IF_VERBOSE_ACTION(log_msg("EX: getlex\t %s, value=%s\n", name.c_str(), val.to_xstring()));

                stack.push(val);
            } break;

        case AS_OPCODE_SetProperty: // setproperty
            {
                int index;
                ip += read_vu30(index, &m_code[ip]);
                const lfl_string& name = m_abc->get_multiname(index);

                IF_VERBOSE_ACTION(log_msg("EX: setproperty\t %s.%s, value=%s\n", stack.top(1).to_xstring(), name.c_str(), stack.top(0).to_xstring()));

                as_object * object = stack.top(1).to_object();

                if( object )
                {
                    object->set_member( name, stack.top(0) );
                }

                stack.drop( 2 );

            } break;

        case AS_OPCODE_GetLocal: // getlocal
            {
                int index;
                ip += read_vu30(index, &m_code[ip]);

                IF_VERBOSE_ACTION(log_msg("EX: getlocal\t index=%i, value=%s\n", index, lregister[index].to_xstring()));

                stack.push(lregister[index]);

            } break;

        case AS_OPCODE_SetLocal: // setlocal
            {
                int index;
                ip += read_vu30(index, &m_code[ip]);

                IF_VERBOSE_ACTION(log_msg("EX: setlocal\t index=%i, value=%s\n", index, stack.top(0).to_xstring()));

                lregister[index] = stack.pop();

            } break;

        case AS_OPCODE_GetScopeObject: // getscopeobject
            {
                int index = m_code[ip];
                ++ip;

                assert( index < scope.size() );

                stack.push( scope[index] );

                IF_VERBOSE_ACTION(log_msg("EX: getscopeobject\t index=%i, value=%s\n", index, stack.top(0).to_xstring()));

            } break;

        case AS_OPCODE_GetProperty:	// getproperty
            {
                int index;
                ip += read_vu30(index, &m_code[ip]);
                lfl_string name = get_multiname(index, stack);

                as_object* obj = stack.top(0).to_object();
                if (obj)
                {
                    obj->get_member(name, &stack.top(0));
                }
                else
                {
                    stack.top(0).set_undefined();
                }

                IF_VERBOSE_ACTION(log_msg("EX: getproperty\t %s, value=%s\n", name.c_str(), stack.top(0).to_xstring()));
            } break;

        case AS_OPCODE_InitProperty:	// initproperty, Initialize a property.
            {
                int index;
                ip += read_vu30(index, &m_code[ip]);
                const lfl_string& name = m_abc->get_multiname(index);

                as_value& val = stack.top(0);
                as_object* obj = stack.top(1).to_object();
                if (obj)
                {
                    obj->set_member(name, val);
                }

                IF_VERBOSE_ACTION(log_msg("EX: initproperty\t 0x%p.%s=%s\n", obj, name.c_str(), val.to_xstring()));

                stack.drop(2);
            } break;

        case AS_OPCODE_Convert_i: //convert_i
            {
                stack.top(0).set_int( stack.top(0).to_int() );
                IF_VERBOSE_ACTION(log_msg("EX: convert_i : %i \n", stack.top(0).to_int())); 
            } break;

        case AS_OPCODE_Convert_b: //convert_i
            {
                as_value& v = stack.top(0);
                v.set_bool(v.to_bool());
                IF_VERBOSE_ACTION(log_msg("EX: convert_b : %s \n", v.to_bool() ? "true" : "false")); 
            }
            break;

        case AS_OPCODE_Coerce: // coerce
            {
                int index;
                ip += read_vu30( index, &m_code[ip]);
                const lfl_string& type_name = m_abc->get_multiname( index );
                IF_VERBOSE_ACTION(log_msg("EX: coerce : %s todo\n", type_name.c_str())); 
            } break;

        case AS_OPCODE_Coerce_s: // coerce_s
            {
                stack.top(0).set_string( stack.top(0).to_string() );
                IF_VERBOSE_ACTION(log_msg("EX: coerce_s : %s\n", stack.top(0).to_string())); 
            } break;

        case AS_OPCODE_Not: // not
            {
                stack.top(0).set_bool( !stack.top(0).to_bool() );

                IF_VERBOSE_ACTION(log_msg("EX: not\n"));

            } break;

        case AS_OPCODE_Add:	// Add two values
            {
                if (stack.top(0).is_string() || stack.top(1).is_string())
                {
                    lfl_string str = stack.top(1).to_string();
                    str += stack.top(0).to_string();
                    stack.top(1).set_lfl_string(str);
                }
                else
                {
                    stack.top(1) += stack.top(0).to_number();
                }
                stack.drop(1);
            } break;

        case AS_OPCODE_Decrement:
        case AS_OPCODE_Decrement_i:
            {
                as_value& operand = stack.top(0);
                operand.set_double(operand.to_number() - 1);
                IF_VERBOSE_ACTION(log_msg("EX: decrement %f\n", operand.to_number()));
            }
            break;

        case AS_OPCODE_Increment:
        case AS_OPCODE_Increment_i:
            {
                as_value& operand = stack.top(0);
                operand.set_double(operand.to_number() + 1);
                IF_VERBOSE_ACTION(log_msg("EX: increment %f\n", operand.to_number()));
            }
            break;

        case AS_OPCODE_Multiply: // multiply
            {
                stack.top(1) = stack.top(1).to_number() * stack.top(0).to_number();
                stack.drop(1);

                IF_VERBOSE_ACTION(log_msg("EX: multiply\n"));
            } break;

        case AS_OPCODE_Equals: // equals
            {
                bool result = as_value::abstract_equality_comparison( stack.top(1), stack.top(0) );

                IF_VERBOSE_ACTION(log_msg("EX: equals %s & %s : %s\n", stack.top(0).to_xstring(), stack.top(1).to_xstring(), result? "true":"false") );

                stack.drop(1);
                stack.top(0).set_bool( result );
            } break;

        case AS_OPCODE_LessThen: //lessthan
            {
                as_value result;
                bool is_nan = as_value::abstract_relational_comparison( stack.top(1), stack.top(0), result);
                UNUSED(is_nan);

                IF_VERBOSE_ACTION(log_msg("EX: lessthan %s & %s : %s\n", stack.top(1).to_xstring(), stack.top(0).to_xstring(), result.to_string() ) );

                stack.drop(1);
                stack.top(0) = result;
            } break;

        case AS_OPCODE_IncLocal_i:  // inclocal_i
            {
                int index;
                ip += read_vu30(index, &m_code[ip]);

                as_value & reg = lregister[ index ];
                reg.set_int( reg.to_int() + 1 );

                IF_VERBOSE_ACTION(log_msg("EX: inclocal_i %i\n", index ) );

            } break;

        case AS_OPCODE_GetLocal_0: // getlocal_0
        case AS_OPCODE_GetLocal_1: // getlocal_1
        case AS_OPCODE_GetLocal_2: // getlocal_2
        case AS_OPCODE_GetLocal_3: // getlocal_3
            {
                as_value& val = lregister[opcode & 0x03];
                stack.push(val);
                IF_VERBOSE_ACTION(log_msg("EX: getlocal_%d\t %s\n", opcode & 0x03, val.to_xstring()));
            } break;

        case AS_OPCODE_SetLocal_0:	// setlocal_0
        case AS_OPCODE_SetLocal_1:	// setlocal_1
        case AS_OPCODE_SetLocal_2:	// setlocal_2
        case AS_OPCODE_SetLocal_3:	// setlocal_3
            {
                lregister[opcode & 0x03] = stack.pop();
                IF_VERBOSE_ACTION(log_msg("EX: setlocal_%d\t %s\n", opcode & 0x03, lregister[opcode & 0x03].to_xstring()));
            } break;

        case AS_OPCODE_DebugFile:
            {
                int fileNameIndex = 0;
                ip += read_vu30(fileNameIndex, &m_code[ip]);
                IF_VERBOSE_ACTION(log_msg("TODO opcode AS_OPCODE_DebugFile\n"));
            }
            break;

        case AS_OPCODE_DebugLine:
            {
                int line = 0;
                ip += read_vu30(line, &m_code[ip]);
                IF_VERBOSE_ACTION(log_msg("TODO opcode AS_OPCODE_DebugLine\n"));
            }
            break;

        case AS_OPCODE_ExeptionTrown:
            {
                IF_VERBOSE_ACTION(log_msg("TODO opcode AS_OPCODE_ExeptionTrown\n"));
            }
            break;

        case AS_OPCODE_Debug:
            {
                ip += 4;
                IF_VERBOSE_ACTION(log_msg("TODO opcode AS_OPCODE_Debug\n"));
            }
            break;

        case AS_OPCODE_Jump:
            {
                int offset = read_signed_offset_24(m_code, ip);
                ip += offset + 3;

                IF_VERBOSE_ACTION(log_msg("EX: jump on %d bytes\n", offset));
            }
            break;

        case AS_OPCODE_Label:
            {
                //noop
                ip;
            }
            break;

        default:
            log_msg("TODO opcode 0x%02X\n", opcode);
            return;
        }
    } while (ip < m_code.size());
}

void as_3_function::read(lfl_stream* in)
    // read method_info
{
    int param_count = in->read_vu30();

    // The return_type field is an index into the multiname
    m_return_type = in->read_vu30();

    m_param_type.resize(param_count);
    for (int i = 0; i < param_count; i++)
    {
        m_param_type[i] = in->read_vu30();
    }

    m_name = in->read_vu30();
    m_flags = in->read_u8();

    if (m_flags & HAS_OPTIONAL)
    {
        int option_count = in->read_vu30();
        m_options.resize(option_count);

        for (int o = 0; o < option_count; ++o)
        {
            m_options[o].m_value = in->read_vu30();
            m_options[o].m_kind = in->read_u8();
        }
    }

    if (m_flags & HAS_PARAM_NAMES)
    {
        /*
        The param_names entry is available only when the HAS_PARAM_NAMES bit is set in the flags. Each param_info
        element of the array is an index into the constant pool’s string array. The parameter name entry exists solely 
        for external tool use and is not used by the AVM2.
        */
        m_param_names.resize(param_count);
        for(int i = 0; i < param_count; ++i)
        {
            m_param_names[i] = in->read_vu30();
        }
    }

    IF_VERBOSE_PARSE(log_msg("method_info: name='%s', type='%s', params=%d\n",
        m_abc->get_string(m_name).c_str(), 
        m_abc->get_multiname(m_return_type).c_str(),
        m_param_type.size()));
}

void as_3_function::read_body(lfl_stream* in)
    // read body_info
{
    IF_VERBOSE_PARSE(log_msg("body_info[%d]\n", m_method));

    m_max_stack = in->read_vu30();
    m_local_count = in->read_vu30();
    m_init_scope_depth = in->read_vu30();
    m_max_scope_depth = in->read_vu30();

    int i, n;
    n = in->read_vu30();	// code_length
    m_code.resize(n);
    for (i = 0; i < n; i++)
    {
        m_code[i] = in->read_u8();
    }

    n = in->read_vu30();	// exception_count
    m_exception.resize(n);
    for (i = 0; i < n; i++)
    {
        as3_exception_info* e = new as3_exception_info();
        e->read(in, m_abc.get_ptr());
        m_exception[i] = e;
    }

    n = in->read_vu30();	// trait_count
    m_trait.resize(n);
    for (int i = 0; i < n; i++)
    {
        traits_info* trait = new traits_info();
        trait->read(in, m_abc.get_ptr());
        m_trait[i] = trait;
    }

    IF_VERBOSE_PARSE(log_msg("method	%i\n", m_method));
    IF_VERBOSE_PARSE(log_disasm_avm2(m_code, m_abc.get_ptr()));

}

lfl_string as_3_function::get_multiname(int index, vm_stack & stack) const
{
    multiname::kind kind = (multiname::kind)m_abc->get_multiname_type( index );
    switch( kind )
    {
    case multiname::CONSTANT_MultinameL :
        assert(stack.top(0).is_string() || stack.top(0).is_number());
        return stack.pop().to_string();
        break;
    case multiname::CONSTANT_Multiname:
    case multiname::CONSTANT_QName:
        return m_abc->get_multiname(index);
    default:
        assert(!"todo");
        return "";
    }
}

int as_3_function::read_signed_offset_24( const membuf& codebuf, int cursor )
{
    int8 s = *reinterpret_cast<int8*>(&m_code[cursor + 2]);

    return m_code[cursor] | m_code[cursor + 1] << 8 | s << 16;
}

}

