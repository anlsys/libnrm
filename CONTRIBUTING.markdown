Contributing to The Project
===========================

The following is a set of guidelines for contributing to this project. These
are guidelines, not rules, so use your best judgement and feel free to propose
changes.

## Commit Messages Styleguide

- use present tense, imperative mood
- reference issues and merge requests
- you can use [Gitlab Flavored Markdown](https://docs.gitlab.com/ee/user/markdown.html)

If you want some help, here is a commit template that you can add to your git
configuration. Save it to a `my-commit-template.txt` file and use `git config
commit.template my-config-template.txt` to use it automatically.

```
# [type] If applied, this commit will...    ---->|


# Why this change

# Links or keys to tickets and others
# --- COMMIT END ---
# Type can be 
#    feature
#    fix (bug fix)
#    doc (changes to documentation)
#    style (formatting, missing semi colons, etc; no code change)
#    refactor (refactoring production code)
#    test (adding missing tests, refactoring tests; no production code change)
# --------------------
# Remember to
#    Separate subject from body with a blank line
#    Limit the subject line to 50 characters
#    Capitalize the subject line
#    Do not end the subject line with a period
#    Use the imperative mood in the subject line
#    Wrap the body at 72 characters
#    Use the body to explain what and why vs. how
#    Can use multiple lines with "-" for bullet points in body
# --------------------
```

## Signoff on Contributions:

The project uses the [Developer Certificate of
Origin](https://developercertificate.org/) for copyright and license
management. We ask that you sign-off all of your commits as certification that
you have the rights to submit this work under the license of the project (in
the `LICENSE` file) and that you agree to the DCO.

To signoff commits, use: `git commit --signoff`.
To signoff a branch after the fact: `git rebase --signoff`
