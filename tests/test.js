var a = 2; 
var arr = new Array();

arr[1] = 7;
print('arr[0] = ' + arr[1]);


function mul(a,b) {
	return a*b;
}

function add (a, b) {
	return 2*(a+mul(2,b)-b) - (a+mul(2,b)-b);
}

function sub (a, b) {
	return a - b;
}


function iter() {
	var i = 0;

	do {
		i = add(i,1.001);
	} while (i < 2000000);
	print(i);
}

var i = 0;



do {
	i = add(i,1.001);
} while (i < 2000000);
print(i);
iter();
