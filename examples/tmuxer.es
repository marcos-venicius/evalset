default_panel = {
  panel = {
    path = "~/",
    cmd = "ls -lah"
  }
}

default_vpanel = {
  vpanel = {
    left = {
      hpanel = {
        top = $/default_panel,
        bottom = $/default_panel
      }
    },
    right = {
      hpanel = {
        top = $/default_panel,
        bottom = $/default_panel
      }
    }
  }
}

sessions = [
  {
    name = "This is a test session",
    windows = [
      {
        name = "testing",
        vpanel = $/default_vpanel
      }
    ]
  },
  {
    name = "This is another test session",
    windows = [
      {
        name = "testing",
        vpanel = $/default_vpanel
      }
    ]
  }
]