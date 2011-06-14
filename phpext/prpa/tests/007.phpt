--TEST--
rpa_dbex_version
--SKIPIF--
<?php if (!extension_loaded("prpa")) print "skip"; ?>
--FILE--
<?php 
echo (rpa_dbex_version());
echo "\n"

?>
--EXPECT--
2.0
