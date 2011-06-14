--TEST--
rpa_dbex_compile($hDbex, $pattern, $myname)
--SKIPIF--
<?php if (!extension_loaded("prpa")) echo "skip\n"; ?>
--FILE--
<?php 
      $hDbex = rpa_dbex_create();
      rpa_dbex_open($hDbex);
      rpa_dbex_load($hDbex, "name ::= [a-z]+");
      rpa_dbex_load($hDbex, "number ::= [0-9]+");
      rpa_dbex_close($hDbex);
      $ret = rpa_dbex_compile($hDbex);
      echo("Compile Returned: ");
      echo($ret);
      echo("\n");
      unset($hDbex);
?>
--EXPECT--
Compile Returned: 0

