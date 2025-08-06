#!/bin/bash

# Ensure we’re on a clean working directory
if ! git diff --quiet || ! git diff --cached --quiet; then
  echo "❌ Working directory is not clean. Commit or stash your changes first."
  exit 1
fi

# Get all local branches starting with src-entrega_
for entrega_branch in $(git branch --format="%(refname:short)" | grep '^src-entrega_'); do
    # Extract the suffix
    suffix="${entrega_branch#src-entrega_}"
    base_branch="src-base_${suffix}"

    # Check if base branch exists
    if git show-ref --verify --quiet "refs/heads/$base_branch"; then
        echo "🔄 Merging $entrega_branch into $base_branch..."

        # Checkout base branch
        git checkout "$base_branch" || { echo "❌ Failed to checkout $base_branch"; exit 1; }

        # Merge entrega into base
        if git merge --no-ff "$entrega_branch" -m "Merge $entrega_branch into $base_branch"; then
            echo "✅ Successfully merged $entrega_branch into $base_branch"
        else
            echo "❌ Merge conflict or failure during merge of $entrega_branch into $base_branch"
            exit 1
        fi
    else
        echo "⚠️ Base branch $base_branch does not exist. Skipping."
    fi
done

echo "🏁 Done."
