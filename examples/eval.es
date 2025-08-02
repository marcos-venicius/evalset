name = "Joseff"
age = 234
bank_account = nil
money = 3.500000
tall = true
small = true
small = false
total = sum(
  10,
  20,
  3,
  6,
  -2,
  sum(1000, -500)
)

floats = sum(
  0.100000, 0.200000,
  sum(3.141500, sum(10, 10)),
  sum(100, -100)
)

double_age = sum($/total, 2)

bills = [
  "Water",
  "Energy",
  $/double_age,
  sum(
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
test_it = $/address["test it"]
items = $/address["bills"]["items"]
array_item_one = $/array[0]["items"][0]

"This should be a valid variable" = ["Hello", "World"]

"Getting index from string-name variable" = $/"This should be a valid variable"[0]
