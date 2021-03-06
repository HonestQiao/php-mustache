
#include "php_mustache.hpp"



// Declarations ----------------------------------------------------------------

PHP_METHOD(MustacheAST, __construct);
PHP_METHOD(MustacheAST, __sleep);
PHP_METHOD(MustacheAST, toArray);
PHP_METHOD(MustacheAST, __toString);
PHP_METHOD(MustacheAST, __wakeup);



// Argument Info ---------------------------------------------------------------

ZEND_BEGIN_ARG_INFO_EX(MustacheAST____construct_args, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 0)
    ZEND_ARG_INFO(0, vars)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(MustacheAST____sleep_args, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(MustacheAST__toArray_args, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(MustacheAST____toString_args, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(MustacheAST____wakeup_args, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 0)
ZEND_END_ARG_INFO()



// Class Entries ---------------------------------------------------------------

zend_class_entry * MustacheAST_ce_ptr;



// Method Entries --------------------------------------------------------------

static zend_function_entry MustacheAST_methods[] = {
  PHP_ME(MustacheAST, __construct, MustacheAST____construct_args, ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
  PHP_ME(MustacheAST, __sleep, MustacheAST____sleep_args, ZEND_ACC_PUBLIC)
  PHP_ME(MustacheAST, toArray, MustacheAST__toArray_args, ZEND_ACC_PUBLIC)
  PHP_ME(MustacheAST, __toString, MustacheAST____toString_args, ZEND_ACC_PUBLIC)
  PHP_ME(MustacheAST, __wakeup, MustacheAST____wakeup_args, ZEND_ACC_PUBLIC)
  { NULL, NULL, NULL }
};



// Helpers ---------------------------------------------------------------------

void mustache_node_from_binary_string(mustache::Node ** node, char * str, int len)
{
  std::vector<uint8_t> uint_str;
  uint_str.resize(len);
  int i = 0;
  for( i = 0; i < len; i++ ) {
    uint_str[i] = str[i];
  }
  
  size_t vpos = 0;
  *node = mustache::Node::unserialize(uint_str, 0, &vpos);
}

void mustache_node_to_binary_string(mustache::Node * node, char ** estr, int * elen)
{
  std::vector<uint8_t> * serialPtr = node->serialize();
  std::vector<uint8_t> & serial = *serialPtr;
  int serialLen = serial.size();
  
  char * str = (char *) emalloc(sizeof(char *) * (serialLen + 1));
  for( int i = 0 ; i < serialLen; i++ ) {
    str[i] = (char) serial[i];
  }
  str[serialLen] = '\0';
  delete serialPtr;
  
  *elen = serialLen;
  *estr = str;
}

void mustache_node_to_zval(mustache::Node * node, zval * current TSRMLS_DC)
{
  zval * children = NULL;
  
  array_init(current);
  
  // Basic data
  add_assoc_long(current, "type", node->type);
  add_assoc_long(current, "flags", node->flags);
  if( NULL != node->data && node->data->length() > 0 ) {
    add_assoc_stringl(current, "data", (char *) node->data->c_str(), node->data->length(), 1);
  }
  
  // Children
  if( node->children.size() > 0 ) {
    ALLOC_INIT_ZVAL(children);
    array_init(children);
    
    mustache::Node::Children::iterator it;
    for ( it = node->children.begin() ; it != node->children.end(); it++ ) {
      zval * child;
      ALLOC_INIT_ZVAL(child);
      mustache_node_to_zval(*it, child TSRMLS_CC);
      add_next_index_zval(children, child);
    }
    
    add_assoc_zval(current, "children", children);
    children = NULL;
  }
  
  // Partials
  if( node->partials.size() > 0 ) {
    ALLOC_INIT_ZVAL(children);
    array_init(children);
    
    mustache::Node::Partials::iterator it;
    for ( it = node->partials.begin() ; it != node->partials.end(); it++ ) {
      zval * child;
      ALLOC_INIT_ZVAL(child);
      mustache_node_to_zval(&(it->second), child TSRMLS_CC);
      add_assoc_zval(children, it->first.c_str(), child);
    }
    
    add_assoc_zval(current, "partials", children);
    children = NULL;
  }
}



// Object Handlers -------------------------------------------------------------

static zend_object_handlers MustacheAST_obj_handlers;

static void MustacheAST_obj_free(void *object TSRMLS_DC)
{
  try {
    php_obj_MustacheAST * payload = (php_obj_MustacheAST *) object;
    
    if( payload->node != NULL ) {
      delete payload->node;
    }
    
    zend_object_std_dtor((zend_object *)object TSRMLS_CC);

    efree(object);
    
  } catch(...) {
    mustache_exception_handler(TSRMLS_C);
  }
}

static zend_object_value MustacheAST_obj_create(zend_class_entry *class_type TSRMLS_DC)
{
  zend_object_value retval;
  
  try {
    php_obj_MustacheAST * payload;
    zval * tmp;

    payload = (php_obj_MustacheAST *) emalloc(sizeof(php_obj_MustacheAST));
    memset(payload, 0, sizeof(php_obj_MustacheAST));
    
    zend_object_std_init((zend_object *) payload, class_type TSRMLS_CC);
    
#if PHP_VERSION_ID < 50399
    zend_hash_copy(payload->obj.properties, &(class_type->default_properties),
        (copy_ctor_func_t) zval_add_ref, NULL, sizeof(zval*));
#else
    object_properties_init(&payload->obj, class_type);
#endif
    
    payload->node = NULL; //new mustache::Node();

    retval.handle = zend_objects_store_put(payload, NULL, (zend_objects_free_object_storage_t) MustacheAST_obj_free, NULL TSRMLS_CC);
    retval.handlers = &MustacheAST_obj_handlers;
    
  } catch(...) {
    mustache_exception_handler(TSRMLS_C);
  }
  
  return retval;
}



// MINIT -----------------------------------------------------------------------

PHP_MINIT_FUNCTION(mustache_ast)
{
  try {
    zend_class_entry ce;

    memcpy(&MustacheAST_obj_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
    MustacheAST_obj_handlers.clone_obj = NULL;
    
    INIT_CLASS_ENTRY(ce, "MustacheAST", MustacheAST_methods);
    ce.create_object = MustacheAST_obj_create;
    
    MustacheAST_ce_ptr = zend_register_internal_class(&ce TSRMLS_CC);
    MustacheAST_ce_ptr->create_object = MustacheAST_obj_create;
    
    zend_declare_property_null(MustacheAST_ce_ptr, ZEND_STRL("binaryString"), ZEND_ACC_PROTECTED TSRMLS_CC);
    
    return SUCCESS;
  } catch(...) {
    mustache_exception_handler(TSRMLS_C);
    return FAILURE;
  }
}



// Methods ---------------------------------------------------------------------

/* {{{ proto void __construct(string binaryString)
   */
PHP_METHOD(MustacheAST, __construct)
{
  try {
    // Custom parameters
    char * str = NULL;
    long str_len = 0;
    
    // Check parameters
    zval * _this_zval = NULL;
    if( zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), (char *) "O|s", 
            &_this_zval, MustacheAST_ce_ptr, &str, &str_len) == FAILURE) {
      throw PhpInvalidParameterException();
    }
    
    // Class parameters
    _this_zval = getThis();
    zend_class_entry * _this_ce = Z_OBJCE_P(_this_zval);
    php_obj_MustacheAST * payload = 
            (php_obj_MustacheAST *) zend_object_store_get_object(_this_zval TSRMLS_CC);
    
    // Check payload
    if( payload->node != NULL ) {
      throw InvalidParameterException("MustacheAST is already initialized");
    }
    
    // Unserialize
    mustache_node_from_binary_string(&payload->node, str, str_len);
    
  } catch(...) {
    mustache_exception_handler(TSRMLS_C);
  }
}
/* }}} __construct */

/* {{{ proto void __sleep()
   */
PHP_METHOD(MustacheAST, __sleep)
{
  try {
    // Check parameters
    zval * _this_zval = NULL;
    if( zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), (char *) "O", 
            &_this_zval, MustacheAST_ce_ptr) == FAILURE) {
      throw PhpInvalidParameterException();
    }
    
    // Class parameters
    _this_zval = getThis();
    zend_class_entry * _this_ce = Z_OBJCE_P(_this_zval);
    php_obj_MustacheAST * payload = 
            (php_obj_MustacheAST *) zend_object_store_get_object(_this_zval TSRMLS_CC);
    
    array_init(return_value);
    
    // Check payload
    if( payload->node != NULL ) {
      // Serialize and store
      char * str = NULL;
      int len = 0;
      mustache_node_to_binary_string(payload->node, &str, &len);
      if( str != NULL ) {
        zend_update_property_stringl(MustacheAST_ce_ptr, _this_zval, 
              ZEND_STRL("binaryString"), str, len TSRMLS_CC);
        add_next_index_string(return_value, "binaryString", 1);
        efree(str);
      }
    }
    
  } catch(...) {
    mustache_exception_handler(TSRMLS_C);
  }
}
/* }}} __sleep */

/* {{{ proto array toArray()
   */
PHP_METHOD(MustacheAST, toArray)
{
  try {
    // Check parameters
    zval * _this_zval = NULL;
    if( zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), (char *) "O", 
            &_this_zval, MustacheAST_ce_ptr) == FAILURE) {
      throw PhpInvalidParameterException();
    }
    
    // Class parameters
    _this_zval = getThis();
    zend_class_entry * _this_ce = Z_OBJCE_P(_this_zval);
    php_obj_MustacheAST * payload = 
            (php_obj_MustacheAST *) zend_object_store_get_object(_this_zval TSRMLS_CC);
    
    // Check payload
    if( payload->node == NULL ) {
      throw InvalidParameterException("MustacheAST was not initialized properly");
    }
    
    // Convert to PHP array
    mustache_node_to_zval(payload->node, return_value TSRMLS_CC);
    
  } catch(...) {
    mustache_exception_handler(TSRMLS_C);
  }
}
/* }}} toArray */

/* {{{ proto string __toString()
   */
PHP_METHOD(MustacheAST, __toString)
{
  try {
    // Check parameters
    zval * _this_zval = NULL;
    if( zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), (char *) "O", 
            &_this_zval, MustacheAST_ce_ptr) == FAILURE) {
      throw PhpInvalidParameterException();
    }
    
    // Class parameters
    _this_zval = getThis();
    zend_class_entry * _this_ce = Z_OBJCE_P(_this_zval);
    php_obj_MustacheAST * payload = 
            (php_obj_MustacheAST *) zend_object_store_get_object(_this_zval TSRMLS_CC);
    
    // Check payload
    if( payload->node == NULL ) {
      throw InvalidParameterException("MustacheAST was not initialized properly");
    }
    
    // Convert to PHP binary string
    char * str = NULL;
    int len = 0;
    mustache_node_to_binary_string(payload->node, &str, &len);
    
    if( str != NULL && len > 0 ) {
      RETURN_STRINGL(str, len, 0);
    }
    
  } catch(...) {
    mustache_exception_handler(TSRMLS_C);
  }
}
/* }}} __toString */


/* {{{ proto void __wakeup()
   */
PHP_METHOD(MustacheAST, __wakeup)
{
  try {
    // Check parameters
    zval * _this_zval = NULL;
    if( zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), (char *) "O", 
            &_this_zval, MustacheAST_ce_ptr) == FAILURE) {
      throw PhpInvalidParameterException();
    }
    
    // Class parameters
    _this_zval = getThis();
    zend_class_entry * _this_ce = Z_OBJCE_P(_this_zval);
    php_obj_MustacheAST * payload = 
            (php_obj_MustacheAST *) zend_object_store_get_object(_this_zval TSRMLS_CC);
    
    // Get object properties
    // @todo should be able to convert this to use zend_hash_find
    int key_type = 0;
    char * key_str = NULL;
    uint key_len = 0;
    ulong key_nindex = 0;
    HashTable * data_hash = NULL;
    HashPosition data_pointer = NULL;
    zval **data_entry = NULL;
    long data_count = 0;
    if( Z_OBJ_HT_P(_this_zval)->get_properties != NULL ) {
      data_hash = Z_OBJ_HT_P(_this_zval)->get_properties(_this_zval TSRMLS_CC);
      data_count = zend_hash_num_elements(data_hash);
    }
    if( data_hash != NULL ) {
      char *prop_name, *class_name;
      zend_hash_internal_pointer_reset_ex(data_hash, &data_pointer);
      while( zend_hash_get_current_data_ex(data_hash, (void**) &data_entry, &data_pointer) == SUCCESS ) {
        if( zend_hash_get_current_key_ex(data_hash, &key_str, &key_len, 
                &key_nindex, false, &data_pointer) == HASH_KEY_IS_STRING ) {
#if PHP_API_VERSION >= 20100412
          zend_unmangle_property_name(key_str, key_len-1, (const char **) &class_name, (const char **) &prop_name);
#else
          zend_unmangle_property_name(key_str, key_len-1, &class_name, &prop_name);
#endif
          if( strcmp(prop_name, "binaryString") == 0 && Z_TYPE_PP(data_entry) == IS_STRING ) {
            if( payload->node != NULL ) {
              delete payload->node;
              payload->node = NULL;
            }
            mustache_node_from_binary_string(&payload->node, Z_STRVAL_PP(data_entry), Z_STRLEN_PP(data_entry));
          }
        }
        zend_hash_move_forward_ex(data_hash, &data_pointer);
      }
    }
    
  } catch(...) {
    mustache_exception_handler(TSRMLS_C);
  }
}
/* }}} __wakeup */
