--TEST--
rpa_dbex_match($hDbex, $pattern, $myname)
--SKIPIF--
<?php if (!extension_loaded("prpa")) echo "skip\n"; ?>
--FILE--
<?php 
	$hDbex = rpa_dbex_create();
	rpa_dbex_open($hDbex);
	rpa_dbex_load($hDbex, "number ::= [0-9]+");
	rpa_dbex_load($hDbex, "name ::= ([a-z] | ' ')+");	
	rpa_dbex_close($hDbex);
	rpa_dbex_compile($hDbex);
	$pattern = rpa_dbex_lookup($hDbex, "name");
	$myname = "Martin Stoilov";
	$stat = rpa_stat_create($hDbex, 0);
	
	$ret = rpa_stat_match($stat, $pattern, RPA_ENCODING_ICASE_UTF8, $myname);
	$matched = "matched: " . substr($myname, 0, $ret) . "\n";
	echo($matched);
	unset($stat);
	unset($hDbex);
      
?>
--EXPECT--
matched: Martin Stoilov
