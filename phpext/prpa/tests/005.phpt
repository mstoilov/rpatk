--TEST--
rpa_dbex_scan
--SKIPIF--
<?php if (!extension_loaded("prpa")) echo "skip\n"; ?>
--FILE--
<?php 

      $hDbex = rpa_dbex_create();
      rpa_dbex_open($hDbex);
      rpa_dbex_load($hDbex, "v ::= xyz");
      rpa_dbex_close($hDbex);
	  rpa_dbex_compile($hDbex);
	  $pattern = rpa_dbex_lookup($hDbex, "v");
      $text = "abcdef xyz aaaaaa";
      $where = 0;
	  $stat = rpa_stat_create($hDbex, 0);
      $ret = rpa_stat_scan($stat, $pattern, RPA_ENCODING_ICASE_UTF8, $text, $where);
      if ($ret >= 0) {
            $found = "found at " .$where . ": " . substr($text, $where, $ret) . "\n";
      	    echo($found);
      }
      unset($hDbex);
?>
--EXPECT--
found at 7: xyz

