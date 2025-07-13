name = "Joseff"
age = 234
bank_account = nil
money = 3.500000
tall = true
small = true
small = false
total = sum_i(
  10,
  20,
  3,
  6,
  -2,
  sum_i(1000, -500)
)

floats = sum_f(
  0.100000, 0.200000,
  sum_f(3.141500, sum_i(10, 10)),
  sum_i(100, -100)
)

double_age = sum_i($/total, 2)

bills = [
  "Water",
  "Energy",
  $/double_age,
  sum_f(
    $/floats,
    $/total,
    $/double_age
  )
]

bills_item_one = $/bills[3]

address = {
  city = "New York",
  state = "NY",
  "country" = "USA",
  "test it" = "testing",
  double_age = $/double_age,
  bills = {
    items = $/bills,
    size = len($/bills)
  }
}

bills_size = len($/bills)

array = [$/address["bills"]]

country_name = $/address["country"]
test_it = $/address["bills"]
items = $/test_it["items"]
array_item_one = $/array[0]

