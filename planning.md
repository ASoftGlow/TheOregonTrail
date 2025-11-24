# List of all
- date
    - month
    - day
- weather
    - percipitation
- supplies
    - sets of clothes
    - food
    - wagon parts
    - medicine
- wagon status
- wagon members' health
- oxen health
- map
- progress

# Cost of information
## Instant
- date
- weather
## Manual
- oxen health
- wagon members' health
- wagon status?
- map
- progress

# Media
| info          | text | icon | bar |
|---------------|------|------|-----|
| date          | x
| weather       | x    | x
| supplies      | x
| wagon status  | x
| member health | x    | x    | x
| oxen health   | x    | x    | x


# Option tree
## Main
- date
- weather
- wagon status - partial
- options
    - continue
    - ## view map
        - options
            - make mark
    - ## go hunting
        - mini-game
    - ## check supplies
        - sets of clothes
        - food
        - wagon parts
        - medicine
    - ## check health
        - options
            - ## oxen
            - ## members
            - give medicine
    - ## check wagon
        - options
            - repair part
            - replace part