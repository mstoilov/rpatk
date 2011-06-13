--TEST--
rpa_dbex_strmatch("martin stoilov", "[a-z]+")
--SKIPIF--
<?php if (!extension_loaded("prpa")) print "skip"; ?>
--FILE--
<?php 
echo (rpa_dbex_strmatch("martin stoilov", "[a-z]+"));
echo "\n"

?>
--EXPECT--
6
