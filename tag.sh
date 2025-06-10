#!/bin/bash

# Check if a version number was provided
if [ -z "$1" ]; then
  echo "Error: No version number provided."
  echo "Usage: ./tag.sh <version>"
  echo "Example: ./tag.sh 1.2.3"
  exit 1
fi

VERSION=$1

if [[ ! "$VERSION" =~ ^[0-9]+\.[0-9]+\.[0-9]+$ ]]; then
  echo "❌ Error: '$VERSION' is not a valid semantic version (X.Y.Z)"
  exit 1
fi

echo "Tagging version $VERSION..."
git tag "$VERSION" || {
  echo "Failed to create tag. Does it already exist?"
  exit 1
}

echo "Pushing tag $VERSION to origin..."
git push origin "$VERSION" || {
  echo "Failed to push tag to origin."
  exit 1
}

echo "Successfully tagged and pushed version $VERSION."