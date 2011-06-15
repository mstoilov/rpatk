--TEST--
rpa_dbex_match($hDbex, $pattern, $myname)
--SKIPIF--
<?php if (!extension_loaded("prpa")) echo "skip\n"; ?>
--FILE--
<?php 
      $hDbex = rpa_dbex_create();
      rpa_dbex_open($hDbex);
      rpa_dbex_load($hDbex, "name ::= [a-z]+");
      rpa_dbex_load($hDbex, "number ::= [0-9]+");
      rpa_dbex_close($hDbex);
      if (rpa_dbex_compile($hDbex) < 0) {
      	 echo("Compile Failed\n");
	 return -1;
	 }
      $pattern = rpa_dbex_lookup($hDbex, "name");
      $myname = "Martin Stoilov";
      $stat = rpa_stat_create($hDbex, 0);
      $ret = rpa_stat_match($stat, $pattern, RPA_ENCODING_ICASE_UTF8, $myname);
      $matched = "matched: " . substr($myname, 0, $ret) . "\n";
      echo($matched);

/*
      for ($p = rpa_dbex_first_pattern($hDbex); $p; $p = rpa_dbex_next_pattern($hDbex, $p)) {
      	  echo(rpa_dbex_pattern_name($hDbex, $p));
	  echo(" ::= ");
      	  echo(rpa_dbex_pattern_regex($hDbex, $p));
	  echo("\n");
      }
*/
      unset($hDbex);
      unset($stat);
?>
--EXPECT--
matched: Martin
name ::= [a-z]+
number ::= [0-9]+
