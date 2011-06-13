--TEST--
rpa_dbex_scan
--SKIPIF--
<?php if (!extension_loaded("prpa")) echo "skip\n"; ?>
--FILE--
<?php 

      $hDbex = rpa_dbex_create();
      rpa_dbex_open($hDbex);
      rpa_dbex_load_string($hDbex, "v ::= xyz");
      rpa_dbex_close($hDbex);
      $pattern = rpa_dbex_get_pattern($hDbex, "v");
      $text = "abcdef xyz aaaaaa";
      $where = 0;
      $ret = rpa_dbex_scan($hDbex, $pattern, $text, $where);
      if ($ret >= 0) {
            $found = "found at " .$where . ": " . substr($text, $where, $ret) . "\n";
      	    echo($found);
      }
      unset($hDbex);
?>
--EXPECT--
found at 7: xyz

