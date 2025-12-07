import type {SidebarsConfig} from '@docusaurus/plugin-content-docs';

/**
 * Creating a sidebar enables you to:
 - create an ordered group of docs
 - render a sidebar for each doc of that group
 - provide next/previous navigation

 The sidebars can be generated from the filesystem, or explicitly defined here.

 Create as many sidebars as you want.
 */
const sidebars: SidebarsConfig = {
  // Main sidebar with organized documentation
  tutorialSidebar: [
    {
      type: 'doc',
      id: 'intro',
      label: '🏠 Home',
    },
    {
      type: 'category',
      label: '📚 Getting Started',
      collapsed: false,
      items: [
        'guides/installation',
        'guides/user-guide',
      ],
    },
    {
      type: 'category',
      label: '🔌 How-To Guides',
      collapsed: false,
      items: [
        'guides/custom-serializers',
      ],
    },
    {
      type: 'category',
      label: '📖 API Reference',
      collapsed: false,
      items: [
        'api/reference',
      ],
    },
    {
      type: 'category',
      label: '⚙️ Architecture & Design',
      collapsed: true,
      items: [
        'internals/architecture',
      ],
    },
    {
      type: 'category',
      label: '🤝 Contributing',
      collapsed: true,
      items: [
        'contributing/developer-guide',
        'contributing/versioning',
      ],
    },
  ],
};

export default sidebars;
