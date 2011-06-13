--TEST--
rpa_dbex_add_callback
--SKIPIF--
<?php if (!extension_loaded("prpa")) echo "skip\n"; ?>
--FILE--
<?php 
      function matched_callback($name, $userdata, $offset, $size, $reason, $input)
      {
        $works = "callback: " . $name . ", " . $reason . ", " . $size . ", " . $input . "\n";
	echo($works);
	return $size;
      }

      $hDbex = rpa_dbex_create();
      rpa_dbex_add_callback($hDbex, "nam.", RPA_REASON_MATCHED, 'matched_callback');
//      rpa_dbex_add_callback($hDbex, "alpha", RPA_REASON_MATCHED, 'matched_callback', "hop hop");
      rpa_dbex_open($hDbex);
      rpa_dbex_load_string($hDbex, "alpha ::= [a-z]");
      rpa_dbex_load_string($hDbex, "name ::= <:alpha:>+");
      rpa_dbex_load_string($hDbex, "number ::= [0-9]+");
      rpa_dbex_load_string($hDbex, "myname ::= <:name:>");
      rpa_dbex_close($hDbex);
      $pattern = rpa_dbex_get_pattern($hDbex, "myname");
      $myname = "martin stoilov";
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
callback: name, 1024, 6, martin stoilov
matched: martin
alpha ::= [a-z]
name ::= <:alpha:>+
number ::= [0-9]+
myname ::= <:name:>
