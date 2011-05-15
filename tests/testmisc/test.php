<?php
	function mul($a, $b)
	{
		return $a*$b;
	}

	function add($a, $b)
	{
		return 2 * ($a + mul(2, $b) - $b) - ($a + mul(2, $b) - $b);
	}

	function iter() {
		$i = 0;
		do {
			$i = add($i, 1.001);
		} while ($i < 2000000);
		echo($i . "\n");
	}

	$i = 0;
	do {
	   $i = add($i, 1.001);
	} while ($i < 2000000);

	echo($i . "\n");
	iter();


?>
