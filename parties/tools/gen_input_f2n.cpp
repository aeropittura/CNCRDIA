#include <iostream>
#include <fstream>
#include "numeric/gf2n.h"
#include "support/buffer.h"

using namespace std;

int main() {
	ifstream cin("gf2n_vals.in");
	ofstream cout("gf2n_vals.out");

	gf2n::init_field(gf2n::default_degree());

	int n; cin >> n;
	for (int i = 0; i < n; ++i) {
		gf2n x; cin >> x;
		cerr << "value is: " << x << "\n";
		x.output(cout,false);
	}
	n = -(n % BUFFER_SIZE) + BUFFER_SIZE;
	cerr << "Adding " << n << " zeros to match buffer size" << endl;
	for (int i = 0; i < n; i++)
		gf2n(0).output(cout, false);

	cin.close();
	cout.close();
	return 0;
}
