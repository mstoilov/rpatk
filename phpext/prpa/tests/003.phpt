--TEST--
rpa_dbex_match($hDbex, $pattern, $myname)
--SKIPIF--
<?php if (!extension_loaded("prpa")) echo "skip\n"; ?>
--FILE--
<?php 
      $hDbex = rpa_dbex_create();
      rpa_dbex_open($hDbex);
      rpa_dbex_load_string($hDbex, "name ::= [a-z]+");
      rpa_dbex_load_string($hDbex, "number ::= [0-9]+");
      rpa_dbex_close($hDbex);
      $pattern = rpa_dbex_get_pattern($hDbex, "name");
      $myname = "Martin Stoilov";
      rpa_dbex_set_encoding($hDbex, RPA_ENCODING_ICASE_UTF8);
      $ret = rpa_dbex_match($hDbex, $pattern, $myname);
      $matched = "matched: " . substr($myname, 0, $ret) . "\n";
      echo($matched);
      for ($p = rpa_dbex_first_pattern($hDbex); $p; $p = rpa_dbex_next_pattern($hDbex, $p)) {
      	  echo(rpa_dbex_pattern_name($hDbex, $p));
	  echo(" ::= ");
      	  echo(rpa_dbex_pattern_regex($hDbex, $p));
	  echo("\n");
      }

      unset($hDbex);
?>
--EXPECT--
matched: Martin
name ::= [a-z]+
number ::= [0-9]+
